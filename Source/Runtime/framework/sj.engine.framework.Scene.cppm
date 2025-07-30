module;

// SJ Includes
#include <ScrewjankStd/TypeMacros.hpp>
#include <ScrewjankStd/Assert.hpp>

// Library Includes
#include <glaze/beve/read.hpp>

export module sj.engine.framework.Scene;
import sj.engine.ecs.components;
import sj.engine.ecs;
import sj.std.memory;
import sj.std.containers.type_list;
import sj.std.containers.vector;
import sj.datadefs;

export namespace sj
{
    using LoadComponentFn = void (*)(ECSRegistry& registry,
                                       GameObjectId goId,
                                       const DataChunk& componentData);

    template<class T>
    void LoadComponent(ECSRegistry& registry, GameObjectId goId, const DataChunk& componentData) 
    {
        T component(componentData);

        // Add new component
        registry.CreateComponent<T>(goId, component);
    };

    // Map chunk type to function that creates runtime component
    constexpr type_map<
        g_componentTypes, 
        TypeId,
        LoadComponentFn,
        []<class T>() -> TypeId{ return T::kChunkTypeId; }, 
        []<class T>() -> LoadComponentFn { return LoadComponent<T>; } > 
        kComponentLoadFns = {};

    class Scene
    {
    public:
        Scene(ECSRegistry& registry, const char* path)
        {
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