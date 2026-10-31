module;
#include <ScrewjankStd/Assert.hpp>

#include <glaze/glaze.hpp>

#include <tuple>
export module sj.engine.TransformComponent;
import sj.std;
import sj.engine.ecs.ECSRegistry;
import sj.engine.ecs.Identifiers;

export namespace sj
{
struct TransformComponent
{
    mat44 localToParent = mat44(kIdentityTag);
};

struct ParentComponent
{
    GameObjectId parentGoId;
};

// struct ChildrenComponent
// {
//     sj::dynamic_vector<GameObjectId> children;
// };

} // namespace sj
