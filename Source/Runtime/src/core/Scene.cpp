// Parent Include
#include <ScrewjankEngine/core/Scene.hpp>

// Engine Includes
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankEngine/core/Game.hpp>

// Shared Includes
#include <ScrewjankShared/io/File.hpp>
#include <ScrewjankShared/DataDefinitions/ScenePrototype.hpp>

// STD Includes
#include <algorithm>

namespace sj
{
    template<class T>
    void LoadComponents(File& sceneFile,
                        const ComponentListHeader& header,
                        IMemSpace* memorySpace,
                        dynamic_vector<T>& out_list)
    {
        // T* buffer = memorySpace->AllocateType<T>(header.numComponents);
        // sceneFile.Read(buffer, sizeof(T) * header.numComponents);

        // out_list = dynamic_vector(memorySpace, buffer, header.numComponents);
    }

    Scene::Scene(const char* path)
    {
        if(path == nullptr)
        {
            return;
        }

        File sceneFile;
        sceneFile.Open(path, sj::File::OpenMode::kReadBinary);

        ScenePrototype sceneProto;
        sceneFile.Read(&sceneProto, sizeof(sceneProto));

        m_memSpace.Init(MemorySystem::GetRootMemSpace(), sceneProto.memory, "Scene Heap");

        size_t scriptPoolMemSize = sceneProto.scriptPoolSize * m_scriptPool.GetBlockSize();
        void* sciptPoolMem = m_memSpace.Allocate(scriptPoolMemSize, m_scriptPool.GetBlockSize());
        m_scriptPool.Init(scriptPoolMemSize, sciptPoolMem);

        {
            MemSpaceScope scope(&m_memSpace);

            // static_assert(sizeof(GameObjectPrototype) == sizeof(GameObject),
            //               "Crimes being done here");


            constexpr size_t kMaxUserDataSize = 128;
            std::array<uint8_t, kMaxUserDataSize> userDataBuffer;

            for(uint32_t i = 0; i < sceneProto.numComponentLists; i++)
            {
                ComponentListHeader header;
                sceneFile.ReadStruct(header);

                switch(header.componentTypeId)
                {
                case CameraComponent::kTypeId:
                    LoadComponents(sceneFile, header, &m_memSpace, m_cameraComponents);
                    break;
                default:
                    SJ_ASSERT(false, "Can't deserialize component type %d", header.componentTypeId);
                    break;
                }
            }
        }

        sceneFile.Close();
    }

    Scene::~Scene()
	{
        m_memSpace.Free(reinterpret_cast<void*>(m_scriptPool.Begin()));
    }

    std::span<CameraComponent> Scene::GetCameraComponents()
    {
        return std::span<CameraComponent>(m_cameraComponents.data(), m_cameraComponents.size());
    }
}