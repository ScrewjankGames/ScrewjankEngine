module;

#include <box3d/types.h>
#include <box3d/id.h>

export module sj.datadefs.components.PhysicsComponents;
import sj.std;

export namespace sj
{
///
/// Helpers
///

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

///
/// Components
///

struct RigidbodyComponent
{
    enum BodyType : uint8_t
    {
        kStatic = b3_staticBody,
        kKinematic = b3_kinematicBody,
        kDynamic = b3_dynamicBody
    };

    BodyType body_type = kStatic;
    vec4 linear_velocity = vec4();
    float gravity_scale = 1.0f;

    b3BodyId runtime_body_id = b3_nullBodyId;
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