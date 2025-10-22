module;

// SJ Includes
#include <ScrewjankStd/Assert.hpp>

// Library Includes
#include <glaze/beve/read.hpp>

export module sj.engine.core.Scene;
import sj.engine.ecs;
import sj.std.memory;
import sj.std.type_info;
import sj.std.containers.type_list;
import sj.std.containers.vector;
import sj.datadefs;

export namespace sj
{
    class Scene
    {
    public:
        Scene(const char* path, ECSRegistry& registry, auto ComponentManifest)
        {
            // Map chunk type to function that creates runtime component
            constinit static type_map<ComponentManifest.GetComponentTypes(),
                               TypeId,
                               LoadComponentFn,
                               []<class T>() -> TypeId {return type_id_of<T>();},
                               []<class T>() -> LoadComponentFn {return LoadComponent<T>;}>
                kComponentLoadFns = {};

            SceneChunk chunk;

            glz::error_ctx errorCtx =
                glz::read_file_beve(chunk, path, sj::dynamic_vector<std::byte> {});
            SJ_ASSERT(errorCtx.ec == glz::error_code::none, "Failed to load scene {0}", path);

            for(const GameObjectChunk& goChunk : chunk.gameObjects)
            {
                GameObjectId goId = registry.CreateGameObject();
                for(const DataChunk& componentChunk : goChunk.components)
                {
                    LoadComponentFn createFn = kComponentLoadFns.get(componentChunk.type);
                    std::invoke(createFn, registry, goId, componentChunk);
                }
            }
        }

        ~Scene() = default;

    private:
        free_list_allocator m_memoryResource;
    };
} // namespace sj