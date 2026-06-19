module;
#include <ScrewjankStd/Assert.hpp>

#include <memory>
#include <memory_resource>
#include <ranges>

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
concept owns_components = requires { is_type_list<decltype(tSystem::kOwnedComponents)>; };

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
          mComponentBindings(mMemoryResource), mComponentCreateCallbacks(mMemoryResource),
          mComponentDestroyCallbacks(mMemoryResource)
    {
    }

    template <class tSystem>
    void RegisterSystem(tSystem* system)
    {
        if constexpr(ecs::owns_components<tSystem>)
        {
            auto registerOwnedComponentFn = []<class tComponent>(ECSRegistry* self,
                                                                 tSystem* system) {
                sj::rtti::register_type<tComponent>();

                if constexpr(ecs::has_on_component_create<tSystem, tComponent>)
                    self->RegisterComponentCreateCallback<tSystem, tComponent>(system);

                if constexpr(ecs::has_on_component_destroy<tSystem, tComponent>)
                    self->RegisterComponentDestroyCallback<tSystem, tComponent>(system);
            };

            tSystem::kOwnedComponents.template for_each<registerOwnedComponentFn>(this, system);
        }
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

            std::invoke(callbackIt->second, goId, archetype->GetEntry(id, goRecord->archetypeLocalIndex));
        }

        archetype->RemoveEntry(goRecord->archetypeLocalIndex);
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

        auto componentBinding = mComponentBindings.find(type_id_of<T>);

        return &archetype->GetRow<T>().at(goIndex);
    }

    template <class T>
    decltype(auto) GetComponents()
    {
        auto componentBinding = mComponentBindings.find(type_id_of<T>);
        SJ_ASSERT(componentBinding != mComponentBindings.end(),
                  "No components of type {} exist",
                  type_name_of<T>);

        auto recordToComponentsFn = [&](ComponentRecord& record) -> std::ranges::range auto {
            Archetype* archetype = GetArchetype(record.archetypeId);
            SJ_ASSERT(archetype,
                      "Component {} is registered to stale archetype id {}",
                      type_name_of<T>,
                      record.archetypeId);

            return std::views::zip(archetype->GetGameObjects(),
                                   archetype->GetRow<T>());
        };

        return componentBinding->second
               | std::views::transform(recordToComponentsFn)
               | std::views::join;
    }

private:
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

            for(const type_info* info : componentTypeInfos)
            {
                auto bindingIt = mComponentBindings.find(info->id);

                if(bindingIt == mComponentBindings.end())
                    bindingIt =
                        mComponentBindings
                            .emplace(info->id, dynamic_vector<ComponentRecord> {mMemoryResource})
                            .first;

                bindingIt->second.emplace_back(ComponentRecord {
                    .archetypeId = aId,
                    .rowIdx = archetype->GetRowIdx(info->id),
                });
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
    dynamic_flat_map<TypeId, dynamic_vector<ComponentRecord>> mComponentBindings;

    // Callbacks
    dynamic_flat_map<TypeId, ComponentEventCallback> mComponentCreateCallbacks;
    dynamic_flat_map<TypeId, ComponentEventCallback> mComponentDestroyCallbacks;
};
} // namespace sj
