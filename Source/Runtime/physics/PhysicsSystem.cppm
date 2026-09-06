module;

#include <ScrewjankStd/Assert.hpp>

#include <box3d/box3d.h>
#include <box3d/types.h>

#include <cmath>
#include <tuple>
#include <ranges>
#include <vector>
#include "box3d/collision.h"

export module sj.engine.physics:PhysicsSystem;
import :Conversions;
import :CollisionTests;
import :RigidbodyComponent;
import :Shapes;

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
    static constexpr type_list<RigidbodyComponent, SphereShape, BoxShape> kRegisteredComponents;
    static constexpr type_list<RigidbodyComponent, SphereShape, BoxShape>
        kRequestedLifetimeCallbacks;

    PhysicsSystem()
    {
        b3WorldDef worldDef = b3DefaultWorldDef();
        mPhysicsWorld = b3CreateWorld(&worldDef);
    }

    ~PhysicsSystem()
    {
        b3DestroyWorld(mPhysicsWorld);
    }

    void Initialize(ECSRegistry& ecs)
    {
        mECS = &ecs;
    }

    void OnCreate(GameObjectId goId, RigidbodyComponent* component)
    {
        b3BodyDef def = b3DefaultBodyDef();

        def.type = static_cast<b3BodyType>(component->body_type);
        def.gravityScale = component->gravity_scale;

        TransformComponent* goTransform = mECS->GetComponent<TransformComponent>(goId);
        SJ_ASSERT(goTransform, "Rigid bodies need transforms");

        def.position = vec4ToB3Vec3(goTransform->localToParent.get_row<3>());
        def.linearVelocity = vec4ToB3Vec3(component->linear_velocity);

        component->runtime_body_id = b3CreateBody(mPhysicsWorld, &def);
    }

    void OnDestroy(GameObjectId goId, RigidbodyComponent* component)
    {
        b3DestroyBody(component->runtime_body_id);
    }

    void OnCreate(GameObjectId goId, SphereShape* component)
    {
        auto* rb = mECS->GetComponent<RigidbodyComponent>(goId);
        TransformComponent* goTransform = mECS->GetComponent<TransformComponent>(goId);

        float scale = goTransform->localToParent.get_unsigned_scale().get_x();

        b3ShapeDef shapeDef = b3DefaultShapeDef();
        shapeDef.filter.categoryBits = static_cast<u64>(component->filter.collision_types);
        shapeDef.filter.maskBits = static_cast<u64>(component->filter.collides_with);

        b3Sphere sphere {.center = vec4ToB3Vec3(component->center),
                         .radius = component->radius * scale};

        component->runtime_shape_id = b3CreateSphereShape(rb->runtime_body_id, &shapeDef, &sphere);
    }

    void OnDestroy(GameObjectId goId, SphereShape* component)
    {
        b3DestroyShape(component->runtime_shape_id, true);
    }

    void OnCreate(GameObjectId goId, BoxShape* component)
    {
        auto* rb = mECS->GetComponent<RigidbodyComponent>(goId);
        TransformComponent* goTransform = mECS->GetComponent<TransformComponent>(goId);

        b3ShapeDef shapeDef = b3DefaultShapeDef();
        shapeDef.filter.categoryBits = static_cast<u64>(component->filter.collision_types);
        shapeDef.filter.maskBits = static_cast<u64>(component->filter.collides_with);

        vec4 scales = goTransform->localToParent.get_unsigned_scale();
        vec4 scaledExtents = component->extents * scales;

        b3BoxHull box =
            b3MakeBoxHull(scaledExtents.get_x(), scaledExtents.get_y(), scaledExtents.get_z());
            
        box.base.center = vec4ToB3Vec3(component->center);
        component->runtime_shape_id = b3CreateHullShape(rb->runtime_body_id, &shapeDef, &box.base);
    }

    void OnDestroy(GameObjectId goId, BoxShape* component)
    {
        b3DestroyShape(component->runtime_shape_id, true);
    }

    void Process(ECSRegistry& ecs, float deltaSeconds)
    {
        mTimeAccumulator += deltaSeconds;

        while(mTimeAccumulator >= mFixedTimeStep)
        {
            b3World_Step(mPhysicsWorld, mFixedTimeStep, 4);
            mTimeAccumulator -= mFixedTimeStep;
        }

        auto bodies = ecs.Query<TransformComponent, RigidbodyComponent>();
        for(auto&& [transform, body] : bodies)
        {
            b3Transform simTransform = b3Body_GetTransform(body.runtime_body_id);
            transform.localToParent =
                b3TransformToMat44(simTransform, transform.localToParent.get_unsigned_scale());
        }
    }

private:
    float mFixedTimeStep = 1.0f / 60.0f;
    float mTimeAccumulator = 0.0f;
    ECSRegistry* mECS = nullptr;
    b3WorldId mPhysicsWorld {};
};

} // namespace sj