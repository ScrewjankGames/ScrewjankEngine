module;

#include <optional>
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
    Scene(std::string_view path, ECSRegistry& registry);
    ~Scene() = default;

private:
    GameObjectId SpawnGameObject(ECSRegistry& registry, std::optional<GameObjectId> parentId, const GameObjectChunk& goChunk );
};
} // namespace sj