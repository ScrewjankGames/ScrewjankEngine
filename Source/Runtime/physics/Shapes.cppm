module;

#include "box3d/id.h"
export module sj.engine.physics:Shapes;
import sj.std;

export namespace sj
{

enum CollisionCategories : u64
{
    kDefault = 1 << 0,
    kPlayer = 1 << 1
};

struct CollisionFilter
{
    CollisionCategories collision_types = kDefault;
    CollisionCategories collides_with = kDefault;
};

struct SphereShape
{
    vec4 center = vec4(); // local to go offset
    CollisionFilter filter {};
    b3ShapeId runtime_shape_id {};
    float radius = 0.0f; // sphere radius
};

struct BoxShape
{
    vec4 center = vec4(); // local to go offset
    vec4 extents = vec4(); // half-lengths of each side
    CollisionFilter filter {};
    b3ShapeId runtime_shape_id {};
};

} // namespace sj