module;
#include <ScrewjankStd/Assert.hpp>

#include <glaze/glaze.hpp>

#include <functional>
#include <vector>
#include <string_view>

module sj.engine.Scene;
import sj.engine.TransformComponent;

namespace sj
{
Scene::Scene(std::string_view path, ECSRegistry& registry)
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
        SpawnGameObject(registry, std::nullopt, goChunk);
    }
}

GameObjectId Scene::SpawnGameObject(ECSRegistry& registry,
                                    std::optional<GameObjectId> parentId,
                                    const GameObjectChunk& goChunk)
{
    auto scope = ThreadContext::GetScratchpad();
    dynamic_vector<const type_info*> infos(&scope);
    infos.reserve(goChunk.components.size() + (parentId.has_value() ? 1 : 0));
    for(auto&& c : goChunk.components)
    {
        const type_info* info = rtti::find_type_info(c.first.get_hash().AsInt());
        SJ_ASSERT(info, "Failed to find type info for chunk {}", c.first.get_string());
        infos.push_back(info);
    }

    dynamic_vector<std::move_only_function<void(typed_ptr)>> deserializationCallbacks(&scope);
    deserializationCallbacks.reserve(goChunk.components.size() + (parentId.has_value() ? 1 : 0));
    for(auto&& [typeInfo, componentChunk] : std::views::zip(infos, goChunk.components))
    {
        deserializationCallbacks.emplace_back([&](typed_ptr dst) {
            std::invoke(typeInfo->desierialize_json_fn, dst.get_ptr(), componentChunk.second);
        });
    }

    if(parentId.has_value())
    {
        infos.push_back(&type_info_of<ParentComponent>);

        deserializationCallbacks.emplace_back([&](typed_ptr dst) {
            dst.as<ParentComponent>()->parentGoId = parentId.value();
        });
    }
    std::vector ids = std::ranges::to<std::vector>(infos | std::views::transform(&type_info::id));


    GameObjectId currGoId = registry.CreateGameObject(infos, deserializationCallbacks);

    for(const GameObjectChunk& childChunk : goChunk.children)
        SpawnGameObject(registry, currGoId, childChunk);

    return currGoId;
}
} // namespace sj