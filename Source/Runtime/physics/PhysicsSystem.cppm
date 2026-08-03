module;

#include <cmath>
#include <tuple>
#include <ranges>
#include <vector>

export module sj.engine.physics:PhysicsSystem;
import :Colliders;
import :CollisionTests;
import :RigidbodyComponent;

import sj.engine.system.threading.ThreadContext;
import sj.engine.TransformComponent;

import sj.engine.debug;
import sj.engine.ecs;
import sj.std;

export namespace sj
{

class PhysicsSystem
{
public:
    static constexpr type_list<RigidbodyComponent, SphereCollider, BoxCollider>
        kRegisteredComponents;

    PhysicsSystem() = default;
    ~PhysicsSystem() = default;

    void Process(ECSRegistry& ecs, float deltaSeconds)
    {
        // Integrate
        {
            auto&& query = ecs.Query<RigidbodyComponent, TransformComponent>();
            for(auto&& [body, transform] : query)
            {
                if(body.apply_gravity)
                    body.acceleration += vec4(0, -9.8, 0, 0);

                body.linear_velocity += body.acceleration * deltaSeconds;

                vec4 pos = transform.localToParent.get_w();
                transform.localToParent.set_w(pos + body.linear_velocity * deltaSeconds);

                body.acceleration = vec4();
            }
        }

        // Handle collisions
        {
            auto&& spheres = ecs.QueryWithIds<TransformComponent, SphereCollider>()
                             | std::ranges::to<std::vector>();

            auto&& boxes = ecs.QueryWithIds<TransformComponent, BoxCollider>()
                           | std::ranges::to<std::vector>();

            auto toSphereFn = [](const TransformComponent& t, const SphereCollider& s) -> Sphere {
                const mat44& iLw = t.localToParent;
                return Sphere {
                    .position = iLw.get_w() + s.center,
                    .radius = s.radius
                              * iLw.get_x().magnitude(), // Assuming uniform scale. TODO: Assert it
                };
            };

            auto toObbFn = [](const TransformComponent& t, const BoxCollider& b) -> OBB {
                return OBB {
                    .wsTransform = t.localToParent.normalized(),
                    .extents = b.extents * t.localToParent.get_unsigned_scale(),
                };
            };

            // Sphere vs sphere
            for(int i = 0; i < spheres.size(); i++)
            {
                auto&& [iGo, iTrans, iSphereComponent] = spheres[i];
                Sphere iSphere = toSphereFn(iTrans, iSphereComponent);

                sj::debug::DrawLine(iSphere.position,
                                    iSphere.position + vec4(0, 1.0, 0, 0) * iSphere.radius,
                                    sj::colors::green);

                sj::debug::DrawLine(iSphere.position,
                                    iSphere.position + vec4(1.0, 0.0, 0, 0) * iSphere.radius,
                                    sj::colors::red);

                sj::debug::DrawLine(iSphere.position,
                                    iSphere.position + vec4(0.0, 0.0, -1.0, 0) * iSphere.radius,
                                    sj::colors::blue);

                for(int j = i + 1; j < spheres.size(); j++)
                {
                    auto&& [jGo, jTrans, jSphereComponent] = spheres[j];
                    Sphere jSphere = toSphereFn(jTrans, jSphereComponent);

                    auto&& collision = TestSphereSphereCollision(iSphere, jSphere);

                    if(collision)
                    {
                        RigidbodyComponent* rbA = ecs.GetComponent<RigidbodyComponent>(iGo);
                        RigidbodyComponent* rbB = ecs.GetComponent<RigidbodyComponent>(jGo);

                        if(rbA)
                            rbA->linear_velocity = vec4();

                        if(rbB)
                            rbB->linear_velocity = vec4();
                    }
                }
            }

            // Sphere vs obb
            for(int i = 0; i < spheres.size(); i++)
            {
                auto&& [iGo, iTrans, iSphereComponent] = spheres[i];
                Sphere sphere = toSphereFn(iTrans, iSphereComponent);

                for(int j = i; j < boxes.size(); j++)
                {
                    auto&& [jGo, jTrans, jBoxComponent] = boxes[j];
                    OBB obb = toObbFn(jTrans, jBoxComponent);

                    auto&& collision = TestSphereOBBCollision(sphere, obb);

                    if(collision)
                    {
                        RigidbodyComponent* rbA = ecs.GetComponent<RigidbodyComponent>(iGo);
                        RigidbodyComponent* rbB = ecs.GetComponent<RigidbodyComponent>(jGo);

                        if(rbA)
                            rbA->linear_velocity = vec4();

                        if(rbB)
                            rbB->linear_velocity = vec4();
                    }
                }
            }
        }
    }

private:
};

} // namespace sj