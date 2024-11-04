#pragma once

// Engine Includes
#include <ScrewjankEngine/system/memory/MemSpace.hpp>
#include <ScrewjankEngine/containers/Vector.hpp>
#include <ScrewjankEngine/core/GameObject.hpp>
#include <ScrewjankEngine/containers/UnorderedMap.hpp>
#include <ScrewjankEngine/system/memory/allocators/PoolAllocator.hpp>

// Shared Includes
#include <ScrewjankShared/DataDefinitions/ScenePrototype.hpp>
#include <ScrewjankShared/DataDefinitions/Components/CameraComponent.hpp>
#include <ScrewjankShared/DataDefinitions/Components/ScriptComponent.hpp>

// STD Includes
#include <span>
#include <memory>

namespace sj
{
    // Forward Declarations
    class GameObject; 
    

    // Collection of all game objects and components.
    // Systems will reach in to get component lists
    class Scene
    {
    public:
        Scene(const char* path);
        ~Scene();

        GameObject* GetGameObject(const GameObjectId& goId) const;

        std::span<CameraComponent> GetCameraComponents();
        std::span<ScriptComponent> GetScriptComponents();

    private:
        MemSpace<sj::FreeListAllocator> m_memSpace;

        dynamic_vector<GameObject> m_gameObjects;
        dynamic_vector<CameraComponent> m_cameraComponents;
        dynamic_vector<ScriptComponent> m_scriptComponents;

        PoolAllocator<64> m_scriptPool;
    };
}