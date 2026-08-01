module;

export module sj.engine.physics:Colliders;
import sj.std;

export namespace sj
{

struct SphereCollider
{
    vec4 center = vec4(); // local to go offset
    float radius = 0.0f; // sphere radius
};

struct BoxCollider
{
    vec4 center = vec4(); // local to go offset
    vec4 extents = vec4(); // half-lengths of each side
};

} // namespace sj