module;

export module sj.engine.physics:RigidbodyComponent;
import sj.std;

export namespace sj
{

struct RigidbodyComponent
{
    vec4 linear_velocity = vec4();
    vec4 acceleration = vec4();
    float mass = 1.0f;
    bool apply_gravity = true;
};

} // namespace sj