module;
#include <ScrewjankStd/Assert.hpp>

#include <algorithm>
#include <memory>
#include <memory_resource>
#include <ranges>
#include <type_traits>

export module sj.engine.ecs.ECSRegistry;

import sj.std;

import sj.engine.ecs.Archetype;
import sj.engine.ecs.Identifiers;

import sj.engine.system.memory.MemorySystem;
import sj.datadefs;

export namespace sj
{
namespace ecs
{
template <class tSystem>
concept registers_components = requires { is_type_list<decltype(tSystem::kRegisteredComponents)>; };

template <class tSystem>
concept registers_lifetime_callbacks =
    requires { is_type_list<decltype(tSystem::kRequestedLifetimeCallbacks)>; };

template <class tSystem, class tComponent>
concept has_on_component_create =
    requires(tSystem sys, GameObjectId id, tComponent p) { sys.OnCreate(id, &p); };

template <class tSystem, class tComponent>
concept has_on_component_destroy =
    requires(tSystem sys, GameObjectId id, tComponent p) { sys.OnCreate(id, &p); };
} // namespace ecs

class ECSRegistry
{
public:
    using ComponentEventCallback = std::move_only_function<void(GameObjectId, typed_ptr)>;

    ECSRegistry()
        : mMemoryResource(sj::MemorySystem::GetRootMemoryResource()),
          mGameObjects(100, mMemoryResource), mArchetypes(mMemoryResource),
          mCachedQueries(mMemoryResource), mComponentCreateCallbacks(mMemoryResource),
          mComponentDestroyCallbacks(mMemoryResource)
    {
    }

    template <class tSystem>
    void RegisterSystem(tSystem* system)
    {
        if constexpr(ecs::registers_components<tSystem>)
            RegisterComponents(system);

        if constexpr(ecs::registers_lifetime_callbacks<tSystem>)
            RegisterComponentLifetimeCallbacks(system);
    }

    template <std::ranges::range ComponentTypeInfos, std::ranges::range ConstructCallbacks>
    GameObjectId CreateGameObject(const ComponentTypeInfos& componentTypeInfos,
                                  ConstructCallbacks&& constructFns)
    {
        ArchetypeId aId = ComputeArchetypeId(componentTypeInfos);
        Archetype* archetype = FindOrAddArchetype(aId, componentTypeInfos);

        GameObjectId newGoId = mGameObjects.create(GameObjectRecord {.archetypeId = aId});
        GameObjectRecord* newRecord = mGameObjects.get<GameObjectRecord>(newGoId);

        const auto entryIdx =
            archetype->AddEntry(newGoId, std::forward<ConstructCallbacks>(constructFns));

        newRecord->archetypeLocalIndex = entryIdx;

        for(const type_info* info : componentTypeInfos)
        {
            auto callbackIt = mComponentCreateCallbacks.find(info->id);
            if(callbackIt == mComponentCreateCallbacks.end())
                continue;

            std::invoke(callbackIt->second, newGoId, archetype->GetEntry(info->id, entryIdx));
        }

        return newGoId;
    }

    void ReleaseGameObject(GameObjectId goId)
    {
        GameObjectRecord* goRecord = mGameObjects.get<GameObjectRecord>(goId);
        SJ_ASSERT(goRecord, "Cannot release missing game object");

        Archetype* archetype = GetArchetype(goRecord->archetypeId);
        SJ_ASSERT(archetype, "Cannot release from null archetype for object ID");

        for(TypeId id : archetype->GetTypeIds())
        {
            auto callbackIt = mComponentDestroyCallbacks.find(id);
            if(callbackIt == mComponentDestroyCallbacks.end())
                continue;

            std::invoke(callbackIt->second,
                        goId,
                        archetype->GetEntry(id, goRecord->archetypeLocalIndex));
        }

        archetype->RemoveEntry(goRecord->archetypeLocalIndex);
    }

    template <class... ComponentTypes>
    std::ranges::range auto Query()
    {
        CachedQuery& query = FindOrAddCachedQuery<ComponentTypes...>();

        using ResultType = std::tuple<GameObjectId, std::add_lvalue_reference<ComponentTypes>...>;
        auto archetypeToTupleRangeFn = [&](Archetype* archetype) {
            SJ_ASSERT(archetype, "Invalid archetype");

            auto&& rows = std::make_tuple(archetype->GetGameObjects(),
                                          archetype->GetRow<ComponentTypes>()...);

            auto&& tupleOfRanges = std::apply(
                [](auto&&... ranges) {
                    return std::views::zip(std::forward<decltype(ranges)>(ranges)...);
                },
                rows);

            return tupleOfRanges;
        };

        return query.matchedArchetypes
               | std::views::transform(archetypeToTupleRangeFn)
               | std::views::join;
    }

    template <class T>
    T* GetComponent(GameObjectId goId)
    {
        const GameObjectRecord* goRecord = mGameObjects.get<GameObjectRecord>(goId);
        SJ_ASSERT(goRecord, "Component lookup on invalid game object!");
        auto&& [archetypeId, goIndex] = *goRecord;

        Archetype* archetype = GetArchetype(goRecord->archetypeId);
        if(!archetype)
            return nullptr;

        return &archetype->GetRow<T>().at(goIndex);
    }

private:
    using QueryId = uint64_t;
    struct CachedQuery
    {
        dynamic_array<TypeId> types;
        dynamic_vector<Archetype*> matchedArchetypes;

        bool MatchesArchetype(Archetype* a)
        {
            return a->MatchesQuery(types);
        }
    };

    struct GameObjectRecord
    {
        ArchetypeId archetypeId;
        size_t archetypeLocalIndex = -1;
    };

    struct ComponentRecord
    {
        ArchetypeId archetypeId;
        Archetype::RowIdx rowIdx;
    };

    Archetype* FindOrAddArchetype(ArchetypeId aId, std::ranges::range auto componentTypeInfos)
    {
        auto archetypeIt = mArchetypes.find(aId);

        if(archetypeIt == mArchetypes.end())
        {
            auto archetype = std::make_unique<Archetype>(componentTypeInfos, mMemoryResource);

            for(CachedQuery& q : mCachedQueries.values())
            {
                if(q.MatchesArchetype(archetype.get()))
                    q.matchedArchetypes.push_back(archetype.get());
            }

            auto insertResult = mArchetypes.emplace(aId, std::move(archetype));
            SJ_ASSERT(insertResult.second == true,
                      "Failed to insert new archetype id {}. It is already present in the map",
                      aId);

            archetypeIt = insertResult.first;
        }

        return archetypeIt->second.get();
    }

    Archetype* GetArchetype(ArchetypeId aId)
    {
        auto archetypeIt = mArchetypes.find(aId);
        if(archetypeIt == mArchetypes.end())
            return nullptr;

        return archetypeIt->second.get();
    }

    template <class... Ts>
    CachedQuery& FindOrAddCachedQuery()
    {
        std::hash<TypeId> hasher;
        QueryId queryId = 0 ^ (... ^ hasher(type_id_of<Ts>));

        auto queryIt = mCachedQueries.find(queryId);
        if(queryIt != mCachedQueries.end())
            return queryIt->second;

        CachedQuery& query = mCachedQueries[queryId];
        {
            query.types.resize(sizeof...(Ts));
            size_t idx = 0;
            (void(query.types[idx++] = type_id_of<Ts>), ...);
        }

        for(std::unique_ptr<Archetype>& archetype : mArchetypes.values())
        {
            if(query.MatchesArchetype(archetype.get()))
                query.matchedArchetypes.push_back(archetype.get());
        }

        return query;
    }

    template <ecs::registers_components tSystem>
    void RegisterComponents(tSystem* system)
    {
        tSystem::kRegisteredComponents
            .template for_each<[]<class tComponent>(ECSRegistry* self, tSystem* system) {
                sj::rtti::register_type<tComponent>();
            }>(this, system);
    }

    template <class tSystem>
    void RegisterComponentLifetimeCallbacks(tSystem* system)
    {
        auto registerCreateDestroy = []<class tComponent>(ECSRegistry* self, tSystem* system) {
            static_assert(ecs::has_on_component_create<tSystem, tComponent>,
                          "System requested lifetime callbacks for component type, but does not "
                          "provide a valid member OnCreate(GameObjectId, ComponentType*) function");
            self->RegisterComponentCreateCallback<tSystem, tComponent>(system);

            static_assert(
                ecs::has_on_component_destroy<tSystem, tComponent>,
                "System requested lifetime callbacks for component type, but does not "
                "provide a valid member OnDestroy(GameObjectId, ComponentType*) function");
            self->RegisterComponentDestroyCallback<tSystem, tComponent>(system);
        };

        tSystem::kRequestedLifetimeCallbacks.template for_each<registerCreateDestroy>(this, system);
    }

    template <class tSystem, class tComponent>
    void RegisterComponentCreateCallback(tSystem* system)
    {
        auto componentCreateWrapperFn = [system](GameObjectId goId, typed_ptr p) {
            SJ_ASSERT(p.is<tComponent>(),
                      "Unexpected component type sent to component create event callback");

            tComponent* ptr = p.as<tComponent>();
            SJ_ASSERT(ptr, "Null component pointer");

            system->OnCreate(goId, ptr);
        };

        mComponentCreateCallbacks[type_id_of<tComponent>] = componentCreateWrapperFn;
    }

    template <class tSystem, class tComponent>
    void RegisterComponentDestroyCallback(tSystem* system)
    {
        auto componentDestroyWrapperFn = [system](GameObjectId goId, typed_ptr p) {
            SJ_ASSERT(p.is<tComponent>(),
                      "Unexpected component type sent to component destroy event callback");

            tComponent* ptr = p.as<tComponent>();
            SJ_ASSERT(ptr, "Null component pointer");

            system->OnDestroy(goId, ptr);
        };

        mComponentDestroyCallbacks[type_id_of<tComponent>] = componentDestroyWrapperFn;
    }

    std::pmr::memory_resource* mMemoryResource = nullptr;

    // Lookup
    sparse_set<GameObjectId, GameObjectRecord> mGameObjects;
    dynamic_flat_map<ArchetypeId, std::unique_ptr<Archetype>> mArchetypes;
    dynamic_flat_map<QueryId, CachedQuery> mCachedQueries;

    // Callbacks
    dynamic_flat_map<TypeId, ComponentEventCallback> mComponentCreateCallbacks;
    dynamic_flat_map<TypeId, ComponentEventCallback> mComponentDestroyCallbacks;
};
} // namespace sj
