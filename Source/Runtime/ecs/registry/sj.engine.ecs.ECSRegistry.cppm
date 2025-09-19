module;
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/TypeMacros.hpp>

#include <memory_resource>
#include <ranges>

export module sj.engine.ecs.ECSRegistry;
export import :Identifiers;

import sj.std.containers.any;
import sj.std.containers.sparse_set;
import sj.std.containers.map;
import sj.engine.ecs.components;
import sj.engine.ecs.ComponentManifest;
import sj.engine.system.memory.MemorySystem;
import sj.datadefs;

export namespace sj
{
    class ECSRegistry
    {
    public:
        ECSRegistry(auto tComponentManifest)
            : m_memoryResource(sj::MemorySystem::GetRootMemoryResource()), m_gameObjects(100, m_memoryResource),
              m_componentPools(m_memoryResource)
        {
            auto registerFn = []<class T>(sj::ECSRegistry& registry) {
                registry.RegisterComponentType<T>();
            };

            tComponentManifest.GetComponentTypes().template for_each<registerFn>(*this);
        }

        ECSRegistry() : ECSRegistry(ComponentManifest{})
        {

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
            pool.create(goId, T {std::forward<Args>(args)...});
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
                T::kTypeId,
                ComponentPool<T>(m_gameObjects.get_sparse_size(), m_memoryResource));
        }

        template <class T>
        auto GetComponents()
        {
            auto& pool = GetComponentPool<T>();
            return pool.get_all();
        }

    private:
        template <class T>
        using ComponentPool = sparse_set<GameObjectId, T>;
        using ComponentPoolHandle = sj::static_any<sizeof(ComponentPool<int>)>;

        template <class T>
        ComponentPool<T>& GetComponentPool()
        {
            ComponentPoolHandle* handle = FindComponentPool(T::kTypeId);
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
    };
} // namespace sj
