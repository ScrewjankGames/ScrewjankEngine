module;
#include <glaze/glaze.hpp>

#include <ScrewjankStd/Assert.hpp>

export module sj.engine.TransformComponent;
import sj.datadefs.DataChunk;
import sj.std.math;
import sj.engine.ecs.ECSRegistry;
import sj.engine.ecs.Identifiers;

export namespace sj
{
struct TransformComponent
{
    Mat44 localToParent = Mat44(kIdentityTag);
};
} // namespace sj
