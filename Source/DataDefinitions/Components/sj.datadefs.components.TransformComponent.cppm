module;
#include <ScrewjankStd/Assert.hpp>

export module sj.datadefs.components.TransformComponent;
import sj.std;

export namespace sj
{
struct TransformComponent
{
    mat44 localToParent = mat44(kIdentityTag);
};

// struct ChildrenComponent
// {
//     sj::dynamic_vector<GameObjectId> children;
// };

} // namespace sj
