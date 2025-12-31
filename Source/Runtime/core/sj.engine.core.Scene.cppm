module;

#include <ScrewjankStd/Assert.hpp>

#include <glaze/glaze.hpp>

#include <vector>
#include <string_view>

export module sj.engine.core.Scene;
import sj.engine.system.threading.ThreadContext;
import sj.engine.ecs;
import sj.std;
import sj.datadefs;

export namespace sj
{
class Scene
{
public:
    Scene(std::string_view path, ECSRegistry& registry, auto ComponentManifest)
    {
        // Map chunk type to function that creates runtime component
        constinit static type_map<ComponentManifest.GetComponentTypes(),
                                  TypeId,
                                  LoadComponentFn,
                                  []<class T>() -> TypeId {
                                      return type_id_of<T>;
                                  },
                                  []<class T>() -> LoadComponentFn {
                                      return LoadComponent<T>;
                                  }>
            kComponentLoadFns = {};

        auto scope = ThreadContext::GetScratchpad();
        std::pmr::vector<char> buffer(&scope.get_allocator());
        SceneChunk chunk;

        glz::error_ctx errorCtx =
            glz::read_file_json<glz::opts {.error_on_unknown_keys = false}>(chunk, path, buffer);
        SJ_ASSERT(errorCtx.ec == glz::error_code::none,
                  "Failed to load scene {0}. Error {1}",
                  path,
                  glz::format_error(errorCtx, buffer));

        for(const GameObjectChunk& goChunk : chunk.game_objects)
        {
            GameObjectId goId = registry.CreateGameObject();
            for(const DataChunk& componentChunk : goChunk.components)
            {
                LoadComponentFn createFn = kComponentLoadFns.get(componentChunk.type.get_hash().AsInt());
                std::invoke(createFn, registry, goId, componentChunk);
            }
        }
    }

    ~Scene() = default;
};
} // namespace sj