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
        GameObject go(goId, *mECS);
        mat44 goTransform = go.GetTransformLW();

        def.position = vec4ToB3Vec3(goTransform.get_row<3>());
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
        GameObject go(goId, *mECS);
        mat44 goTransform = go.GetTransformLW();

        float scale = goTransform.get_unsigned_scale().get_x();

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
        GameObject go(goId, *mECS);
        auto* rb = go.GetComponent<RigidbodyComponent>();
        mat44 goTransform = go.GetTransformLW();

        b3ShapeDef shapeDef = b3DefaultShapeDef();
        shapeDef.filter.categoryBits = static_cast<u64>(component->filter.collision_types);
        shapeDef.filter.maskBits = static_cast<u64>(component->filter.collides_with);

        vec4 scales = goTransform.get_unsigned_scale();
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

        auto bodies = ecs.QueryWithIds<TransformComponent, RigidbodyComponent>();
        for(auto&& [goId, transform, body] : bodies)
        {
            GameObject go(goId, ecs);
            b3Transform simTransform = b3Body_GetTransform(body.runtime_body_id);
            go.SetTransformLW(
                b3TransformToMat44(simTransform, go.GetTransformLW().get_unsigned_scale()));
        }
    }

private:
    float mFixedTimeStep = 1.0f / 60.0f;
    float mTimeAccumulator = 0.0f;
    ECSRegistry* mECS = nullptr;
    b3WorldId mPhysicsWorld {};
};

} // namespace sj