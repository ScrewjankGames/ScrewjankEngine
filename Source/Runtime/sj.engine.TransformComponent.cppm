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
    mat44 localToParent = mat44(kIdentityTag);
};
} // namespace sj
