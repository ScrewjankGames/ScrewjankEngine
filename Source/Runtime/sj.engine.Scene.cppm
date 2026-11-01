module;

#include <optional>
#include <string_view>
#include <span>

export module sj.engine.Scene;
import sj.engine.system.threading.ThreadContext;
import sj.engine.ecs;
import sj.std;
import sj.datadefs.ChunkReader;

export namespace sj
{

class Scene
{
public:
    Scene(std::string_view path, ECSRegistry& registry);
    ~Scene() = default;

private:
    void LoadGameObject(ChunkReader& reader,
                        ECSRegistry& registry,
                        std::optional<GameObjectId> parentId);
};
} // namespace sj