#pragma once

// Engine Includes
#include <ScrewjankEngine/system/memory/MemSpace.hpp>
#include <ScrewjankEngine/system/memory/allocators/PoolAllocator.hpp>

// Shared Includes
#include <ScrewjankShared/DataDefinitions/ScenePrototype.hpp>

// STD Includes
#include <span>

import sj.shared.containers;
import sj.shared.datadefs;

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

        std::span<CameraComponent> GetCameraComponents();

    private:
        MemSpace<sj::FreeListAllocator> m_memSpace;

        dynamic_vector<CameraComponent> m_cameraComponents;

        PoolAllocator<64> m_scriptPool;
    };
}