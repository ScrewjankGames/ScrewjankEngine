module;

#include <box3d/types.h>
#include <box3d/id.h>

export module sj.engine.physics:RigidbodyComponent;
import sj.std;

export namespace sj
{

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

} // namespace sj