// Parent Include
#include <ScrewjankEngine/core/Scene.hpp>

// Engine Includes
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankEngine/core/Game.hpp>

// Shared Includes
#include <ScrewjankShared/io/File.hpp>
#include <ScrewjankShared/DataDefinitions/ScenePrototype.hpp>
#include <ScrewjankShared/DataDefinitions/Components/ScriptComponent.hpp>

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

        size_t scriptPoolMemSize = sceneProto.scriptPoolSize * m_scriptPool.GetBlockSize();
        void* sciptPoolMem = m_memSpace.Allocate(scriptPoolMemSize, m_scriptPool.GetBlockSize());
        m_scriptPool.Init(scriptPoolMemSize, sciptPoolMem);

        {
            MemSpaceScope scope(&m_memSpace);
            m_gameObjects.resize(sceneProto.numGameObjects);

            static_assert(sizeof(GameObjectPrototype) == sizeof(GameObject),
                          "Crimes being done here");

            sceneFile.Read(m_gameObjects.data(),
                           sizeof(GameObjectPrototype),
                           sceneProto.numGameObjects);


            const ScriptFactory& scriptFactory = Game::GetScriptFactory();

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
                case ScriptComponent::kTypeId:
                    LoadComponents(sceneFile, header, &m_memSpace, m_scriptComponents);

                    for(ScriptComponent& component : m_scriptComponents)
                    {
                        const ScriptFactoryFn* createFn =
                            scriptFactory.GetScriptCreateFn(component.scriptTypeId);

                        SJ_ASSERT(createFn, "Failed to look up script creatue function");

                        component.userScript = (*createFn)(&m_scriptPool);

                        SJ_ASSERT(component.userDataSize < kMaxUserDataSize,
                                  "User data too big to deserialize");

                        sceneFile.Read(userDataBuffer.data(), component.userDataSize);
                        component.userScript->Deserialize(userDataBuffer.data(),
                                                          component.userDataSize);
                    }

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
        for(ScriptComponent& component : m_scriptComponents)
        {
            m_scriptPool.Free(component.userScript);
        }

        m_memSpace.Free(reinterpret_cast<void*>(m_scriptPool.Begin()));
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

    std::span<ScriptComponent> Scene::GetScriptComponents()
    {
        return m_scriptComponents;
    }
}