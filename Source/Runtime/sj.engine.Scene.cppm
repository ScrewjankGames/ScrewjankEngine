module;

#include <ScrewjankStd/Assert.hpp>

#include <glaze/glaze.hpp>

#include <functional>
#include <vector>
#include <string_view>

export module sj.engine.Scene;
import sj.engine.system.threading.ThreadContext;
import sj.engine.ecs;
import sj.std;
import sj.datadefs;

export namespace sj
{

class Scene
{
public:
    Scene(std::string_view path, ECSRegistry& registry)
    {
        auto scope = ThreadContext::GetScratchpad();
        std::pmr::vector<char> buffer(&scope);
        SceneChunk chunk;

        glz::error_ctx errorCtx =
            glz::read_file_json<glz::opts {.error_on_unknown_keys = false}>(chunk, path, buffer);
        SJ_ASSERT(errorCtx.ec == glz::error_code::none,
                  "Failed to load scene {0}. Error {1}",
                  path,
                  glz::format_error(errorCtx, buffer));

        for(const GameObjectChunk& goChunk : chunk.game_objects)
        {
            dynamic_vector<const type_info*> infos =
                goChunk.components
                | std::views::transform([](const ComponentChunk& chunk) -> const type_info* {
                      const type_info* info = rtti::find_type_info(chunk.first.get_hash().AsInt());
                      SJ_ASSERT(info, "Failed to find type info for chunk {}", chunk.first.get_string());
                      return info;
                  })
                | std::ranges::to<dynamic_vector<const type_info*>>(&scope);

            auto&& deserializeFns =
                std::views::zip(infos, goChunk.components)
                | std::views::transform(
                    [](auto&& pair) -> std::move_only_function<void(typed_ptr)> {
                        auto&& [typeInfo, componentChunk] = pair;
                        return [&](typed_ptr dst) {
                            std::invoke(typeInfo->desierialize_json_fn,
                                        dst.get_ptr(),
                                        componentChunk.second);
                        };
                    });

            registry.CreateGameObject(infos, deserializeFns);
        }
    }

    ~Scene() = default;
};
} // namespace sj