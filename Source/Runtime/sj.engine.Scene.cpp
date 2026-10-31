module;
#include <ScrewjankStd/Assert.hpp>

#include <glaze/glaze.hpp>

#include <functional>
#include <string>

module sj.engine.Scene;
import sj.engine.TransformComponent;

namespace sj
{

Scene::Scene(std::string_view path, ECSRegistry& registry)
{
    auto scope = ThreadContext::GetScratchpad();
    ChunkReader reader(path, &scope);

    SceneChunkHeader sceneHeader = reader.Read<SceneChunkHeader>();

    for(u32 i = 0; i < sceneHeader.num_game_objects; ++i)
    {
        LoadGameObject(reader, registry, std::nullopt);
    }
}

void Scene::LoadGameObject(ChunkReader& reader,
                           ECSRegistry& registry,
                           std::optional<GameObjectId> parentId)
{
    GameObjectChunkHeader goChunk = reader.Read<GameObjectChunkHeader>();

    auto scope = ThreadContext::GetScratchpad();
    dynamic_vector<std::span<const u8>> componentChunks(&scope);

    ComponentChunkHeader componentsHeader = reader.Read<ComponentChunkHeader>();
    for(int i = 0; i < componentsHeader.load_chunk_type_ids.size(); i++)
        componentChunks.emplace_back(reader.ReadUntyped());

    GameObjectId goId =
        registry.CreateGameObject(componentsHeader.load_chunk_type_ids, componentChunks, parentId);

    if(goChunk.num_children > 0)
    {
        for(int i = 0; i < goChunk.num_children; i++)
        {
            LoadGameObject(reader, registry, goId);
        }
    }
}

} // namespace sj