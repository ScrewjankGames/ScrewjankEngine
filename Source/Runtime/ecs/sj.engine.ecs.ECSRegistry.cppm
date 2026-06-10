module;
#include <ScrewjankStd/Assert.hpp>

#include <memory_resource>
#include <ranges>

export module sj.engine.ecs.ECSRegistry;

import sj.std;

import sj.engine.ecs.ComponentManifest;
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
    using ComponentEventCallback = std::function<void(GameObjectId, typed_ptr)>;

    ECSRegistry(auto tComponentManifest)
        : m_memoryResource(sj::MemorySystem::GetRootMemoryResource()),
          m_gameObjects(100, m_memoryResource), m_componentPools(m_memoryResource),
          mComponentCreateCallbacks(m_memoryResource), mComponentDestroyCallbacks(m_memoryResource)

    {
        auto registerFn = []<class T>(sj::ECSRegistry& registry) {
            registry.RegisterComponentType<T>();
        };

        tComponentManifest.GetComponentTypes().template for_each<registerFn>(*this);
    }

    ECSRegistry() : ECSRegistry(ComponentManifest {})
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

    GameObjectId CreateGameObject()
    {
        return m_gameObjects.create();
    }

    void ReleaseGameObject(GameObjectId go)
    {
        m_gameObjects.release(go);
    }

    template <class T, class... Args>
    void CreateComponent(GameObjectId goId, Args&&... args)
    {
        ComponentPool<T>& pool = GetComponentPool<T>();
        auto id = pool.create(goId, T {std::forward<Args>(args)...});

        auto createCallbackIt = mComponentCreateCallbacks.find(type_id_of<T>);
        if(createCallbackIt != mComponentCreateCallbacks.end())
        {
            T* componentPtr = pool.template get<T>(id);
            createCallbackIt->second(goId, componentPtr);
        }
    }

    template <class T>
    T* GetComponent(GameObjectId goId)
    {
        ComponentPool<T>& pool = GetComponentPool<T>();
        return pool.template get<T>(goId);
    }

    template <class T>
    void RegisterComponentType()
    {
        m_componentPools.emplace(
            type_id_of<T>,
            ComponentPool<T>(m_gameObjects.get_sparse_size(), m_memoryResource));
    }

    template <class T>
    auto GetComponents()
    {
        auto& pool = GetComponentPool<T>();
        return pool.get_all();
    }

private:
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

    template <class T>
    using ComponentPool = sparse_set<GameObjectId, T>;
    using ComponentPoolHandle = sj::static_any<sizeof(ComponentPool<int>)>;

    template <class T>
    ComponentPool<T>& GetComponentPool()
    {
        ComponentPoolHandle* handle = FindComponentPool(type_id_of<T>);
        SJ_ASSERT(handle != nullptr, "Failed to find component pool!");

        return handle->get<ComponentPool<T>>();
    }

    auto FindComponentPool(TypeId typeId) -> ComponentPoolHandle*
    {
        const auto& componentPoolIt = m_componentPools.find(typeId);
        if(componentPoolIt == m_componentPools.end())
            return nullptr;

        ComponentPoolHandle& handle = componentPoolIt->second;
        return &handle;
    }

    std::pmr::memory_resource* m_memoryResource = nullptr;
    sparse_set<GameObjectId> m_gameObjects;
    dynamic_flat_map<TypeId, ComponentPoolHandle> m_componentPools;

    dynamic_flat_map<TypeId, ComponentEventCallback> mComponentCreateCallbacks;
    dynamic_flat_map<TypeId, ComponentEventCallback> mComponentDestroyCallbacks;
};
} // namespace sj
