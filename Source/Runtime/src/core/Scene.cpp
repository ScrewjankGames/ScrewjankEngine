// Parent Include
#include <ScrewjankEngine/core/Scene.hpp>

// Engine Includes
#include <ScrewjankEngine/system/memory/Memory.hpp>

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
        T* buffer = memorySpace->AllocateType<T>(header.numComponents);
        sceneFile.Read(buffer, sizeof(T) * header.numComponents);

        out_list = dynamic_vector(buffer, header.numComponents);
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

        {
            MemSpaceScope scope(&m_memSpace);
            m_gameObjects.resize(sceneProto.numGameObjects);

            static_assert(sizeof(GameObjectPrototype) == sizeof(GameObject),
                          "Crimes being done here");

            sceneFile.Read(m_gameObjects.data(),
                           sizeof(GameObjectPrototype),
                           sceneProto.numGameObjects);


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
    }

    GameObject* Scene::GetGameObject(const GameObjectId& goId) const
    {
        auto res = std::ranges::find(m_gameObjects, goId, [](const GameObject& go) {
            return go.GetGoId();
        });

        return res != m_gameObjects.end() ? &(*res) : nullptr;
    }

    std::span<CameraComponent> Scene::GetCameraComponents()
    {
        return std::span<CameraComponent>(m_cameraComponents.data(), m_cameraComponents.size());
    }
}