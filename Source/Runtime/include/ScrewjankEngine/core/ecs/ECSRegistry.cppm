module;
#include <memory_resource>
#include <ScrewjankShared/string/StringHash.hpp>
#include <any>
export module sj.core.ecs:ECSRegistry;
import sj.shared.containers;
import sj.shared.datadefs;
import :Identifiers;

export namespace sj
{
    class ECSRegistry
    {
    public:
        ECSRegistry(uint32_t initialEntityCount, std::pmr::memory_resource* resource)
            : m_gameObjects(initialEntityCount, resource)
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

        template<class ComponentType>
        void RegisterComponentType()
        {
            m_componentPools.emplace(
                {
                    ComponentType::kTypeId, 
                    ComponentPool<ComponentType>(m_gameObjects.get_sparse_size(), m_memoryResource)
                }
            );
        }

        
        template<class ComponentType, class ... Args>
        ComponentType& RegisterComponent(GameObjectId goId, Args&& ... args)
        {
        }

    private:
        template<class T>
        using ComponentPool = sparse_set<GameObjectId, T>;

        std::pmr::memory_resource* m_memoryResource = nullptr;
        sparse_set<GameObjectId> m_gameObjects;
        dynamic_flat_map<TypeId, std::any> m_componentPools;
    };
}

