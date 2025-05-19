module;
#include <memory_resource>
#include <ScrewjankShared/string/StringHash.hpp>
#include <ScrewjankShared/utils/Assert.hpp>

export module sj.engine.framework.ecs:ECSRegistry;
import :Identifiers;

import sj.shared.containers;
import sj.shared.datadefs;

export namespace sj
{
    class ECSRegistry
    {
    public:
        ECSRegistry(uint32_t initialEntityCount, std::pmr::memory_resource* resource)
            : m_memoryResource(resource), 
              m_gameObjects(initialEntityCount, resource),
              m_componentPools(resource)
        {

        }

        GameObjectId CreateGameObject()
        {
            return m_gameObjects.create();
        }

        template<ComponentType T>
        T* GetComponent(GameObjectId goId)
        {
            ComponentPool<T>& pool = GetComponentPool<T>();
            return pool.template get<T>(goId);
        }

        void ReleaseGameObject(GameObjectId go)
        {
            m_gameObjects.release(go);
        }

        template<ComponentType T>
        void RegisterComponentType()
        {
            m_componentPools.emplace(
                    T::kTypeId, 
                    ComponentPool<T>(m_gameObjects.get_sparse_size(), m_memoryResource)
            );
        }
        
        template<ComponentType T, class ... Args>
        void RegisterComponent(GameObjectId goId, Args&& ... args)
        {
            ComponentPool<T>& pool = GetComponentPool<T>();
            pool.create(goId, T{std::forward<Args>(args)...});
        }

    private:
        template<class T>
        using ComponentPool = sparse_set<GameObjectId, T>;
        using ComponentPoolHandle = sj::static_any<sizeof(ComponentPool<int>)>;
        
        template<ComponentType T>
        ComponentPool<T>& GetComponentPool()
        {
            const auto& componentPoolIt = m_componentPools.find(T::kTypeId);
            SJ_ASSERT(componentPoolIt != m_componentPools.end(), "Failed to find pool for component. Please call RegisterComponentType() first");

            ComponentPoolHandle& handle = componentPoolIt->second;
            auto& pool = handle.get<ComponentPool<T>>();
            return pool;
        }

        std::pmr::memory_resource* m_memoryResource = nullptr;
        sparse_set<GameObjectId> m_gameObjects;
        dynamic_flat_map<TypeId, ComponentPoolHandle> m_componentPools;
    };
}

