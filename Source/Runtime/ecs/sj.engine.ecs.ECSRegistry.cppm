module;
#include <ScrewjankStd/Assert.hpp>

#include <algorithm>
#include <vector>
#include <memory>
#include <memory_resource>
#include <optional>
#include <type_traits>
#include <ranges>

export module sj.engine.ecs.ECSRegistry;

import sj.std;

import sj.engine.ecs.Archetype;
import sj.engine.ecs.Identifiers;

import sj.engine.system.memory.MemorySystem;

export namespace sj
{

class ECSRegistry;
class GameObject
{
public:
    GameObject(GameObjectId id, ECSRegistry& r) : mGoId(id), mRegistry(&r)
    {
    }

    [[nodiscard]] mat44 GetTransformLS() const;
    void SetTransformLS(const mat44& m);

    [[nodiscard]] mat44 GetTransformLW() const;
    void SetTransformLW(const mat44& m);

    template <class T>
    T* GetComponent();

private:
    GameObjectId mGoId;
    ECSRegistry* mRegistry;
};

namespace ecs
{
template <class tSystem>
concept registers_lifetime_callbacks =
    requires { is_type_list<decltype(tSystem::kRequestedLifetimeCallbacks)>; };

template <class tSystem, class tComponent>
concept has_on_component_create =
    requires(tSystem sys, GameObject go, tComponent p) { sys.OnCreate(go, &p); };

template <class tSystem, class tComponent>
concept has_on_component_destroy =
    requires(tSystem sys, GameObject go, tComponent p) { sys.OnDestroy(go, &p); };
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
        if constexpr(ecs::registers_lifetime_callbacks<tSystem>)
            RegisterComponentLifetimeCallbacks(system);
    }

    GameObjectId CreateGameObject(std::span<const TypeId> typeIds, std::span<const std::span<const u8>> chunks, std::optional<GameObjectId> parent);
    void ReleaseGameObject(GameObjectId goId);

    template <class... ComponentTypes>
    std::ranges::range auto Query()
    {
        CachedQuery& query = FindOrAddCachedQuery<ComponentTypes...>();

        using ResultType = std::tuple<std::add_lvalue_reference<ComponentTypes>...>;
        auto archetypeToRangeOfTuplesFn = [&](Archetype* archetype) {
            SJ_ASSERT(archetype, "Invalid archetype");

            auto&& [...rows] = std::make_tuple(archetype->GetRow<ComponentTypes>()...);
            return std::views::zip(std::forward<decltype(rows)>(rows)...);
        };

        return query.matchedArchetypes
               | std::views::transform(archetypeToRangeOfTuplesFn)
               | std::views::join;
    }

    template <class... ComponentTypes>
    std::ranges::range auto QueryWithIds()
    {
        CachedQuery& query = FindOrAddCachedQuery<ComponentTypes...>();

        using ResultType = std::tuple<GameObjectId, std::add_lvalue_reference<ComponentTypes>...>;
        auto archetypeToRangeOfTuplesFn = [](Archetype* archetype) {
            SJ_ASSERT(archetype, "Invalid archetype");

            auto&& [...rows] = std::make_tuple(archetype->GetGameObjects(), archetype->GetRow<ComponentTypes>()...);
            return std::views::zip(std::forward<decltype(rows)>(rows)...);
        };

        return query.matchedArchetypes
               | std::views::transform(archetypeToRangeOfTuplesFn)
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

        auto&& row = archetype->GetRow<T>();

        if(row.empty())
            return nullptr;

        return &row[goIndex];
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

    Archetype* FindOrAddArchetype(ArchetypeId aId, std::ranges::range auto componentTypeInfos)
    {
        auto archetypeIt = mArchetypes.find(aId);

        if(archetypeIt == mArchetypes.end())
        {
            auto archetype = std::make_unique<Archetype>(componentTypeInfos, mMemoryResource);

            for(auto&& [id, q] : mCachedQueries)
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

    Archetype* GetArchetype(ArchetypeId aId);

    template <class... Ts>
    CachedQuery& FindOrAddCachedQuery()
    {
        std::hash<TypeId> hasher;
        QueryId queryId = 0 ^ (... ^ hasher(type_id_of<Ts>));

        auto queryIt = mCachedQueries.find(queryId);
        if(queryIt != mCachedQueries.end())
            return queryIt->second;

        queryIt = mCachedQueries.emplace(queryId, CachedQuery()).first;
        CachedQuery& query = queryIt->second;
        {
            query.types.resize(sizeof...(Ts));
            size_t idx = 0;
            (void(query.types[idx++] = type_id_of<Ts>), ...);
        }

        for(auto&& [aId, archetype] : mArchetypes)
        {
            if(query.MatchesArchetype(archetype.get()))
                query.matchedArchetypes.push_back(archetype.get());
        }

        return query;
    }

    template <class tSystem>
    void RegisterComponentLifetimeCallbacks(tSystem* system)
    {
        auto registerCreateDestroy = []<class tComponent>(ECSRegistry* self, tSystem* system) {
            static_assert(ecs::has_on_component_create<tSystem, tComponent>,
                          "System requested lifetime callbacks for component type, but does not "
                          "provide a valid member OnCreate(GameObject, ComponentType*) function");
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
        auto componentCreateWrapperFn = [self = this, system](GameObjectId goId, typed_ptr p) {
            GameObject go(goId, *self);
            SJ_ASSERT(p.is<tComponent>(),
                      "Unexpected component type sent to component create event callback");

            tComponent* ptr = p.as<tComponent>();
            SJ_ASSERT(ptr, "Null component pointer");

            system->OnCreate(go, ptr);
        };

        mComponentCreateCallbacks[type_id_of<tComponent>] = componentCreateWrapperFn;
    }

    template <class tSystem, class tComponent>
    void RegisterComponentDestroyCallback(tSystem* system)
    {
        auto componentDestroyWrapperFn = [self = this, system](GameObjectId goId, typed_ptr p) {
            GameObject go(goId, *self);

            SJ_ASSERT(p.is<tComponent>(),
                      "Unexpected component type sent to component destroy event callback");

            tComponent* ptr = p.as<tComponent>();
            SJ_ASSERT(ptr, "Null component pointer");

            system->OnDestroy(go, ptr);
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

template <class T>
T* GameObject::GetComponent()
{
    return mRegistry->GetComponent<T>(mGoId);
}

} // namespace sj
