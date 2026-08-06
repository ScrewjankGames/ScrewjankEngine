module;

#include <ScrewjankStd/Assert.hpp>

#include <algorithm>
#include <optional>

export module sj.engine.physics:CollisionTests;
import sj.std;

export namespace sj
{

struct Sphere
{
    vec4 position = vec4();
    float radius = 0.0f; // sphere radius
};

struct OBB
{
    mat44 wsTransform = kIdentityTag;
    vec4 extents = vec4();
};

struct Collision
{
    vec4 position = vec4();
    vec4 normal = vec4();

};

std::optional<Collision> TestSphereSphereCollision(const Sphere& a, const Sphere& b)
{
    const float distanceSqr = (a.position - b.position).magnitude_sqr();
    const float combinedRadius = a.radius + b.radius;
    const float combinedRadiusSqr = combinedRadius + combinedRadius;

    if(distanceSqr > combinedRadiusSqr)
        return std::nullopt; // No collision

    Collision c;
    // TODO
    return c;
}

std::optional<Collision> TestSphereOBBCollision(const Sphere& s, const OBB& obb)
{
    SJ_ASSERT(s.position.get_w() == 1.0f, "Invalid W component for sphere position");

    mat44 worldToObb = obb.wsTransform.affine_inverse();
    vec4 spherePosObbSpace = s.position * worldToObb;

    vec4 closestPtObbSurface =
        vec4(std::clamp(spherePosObbSpace.get_x(), -obb.extents.get_x(), obb.extents.get_x()),
             std::clamp(spherePosObbSpace.get_y(), -obb.extents.get_y(), obb.extents.get_y()),
             std::clamp(spherePosObbSpace.get_z(), -obb.extents.get_z(), obb.extents.get_z()),
             1.0f);

    float sphereOffsetSqr = (spherePosObbSpace - closestPtObbSurface).magnitude_sqr();
    if(sphereOffsetSqr > (s.radius * s.radius))
        return std::nullopt; // No collision

    Collision c;
    // TODO
    return c;
}

} // namespace sj