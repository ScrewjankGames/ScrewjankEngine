module;

#include <ScrewjankStd/Assert.hpp>

module sj.engine.ecs.GameObject;
import sj.engine.TransformComponent;

namespace sj
{
    mat44 GameObject::GetTransformLW() const
    {
        TransformComponent* transform = mRegistry->GetComponent<TransformComponent>(mGoId);
        SJ_ASSERT(transform, "GameObject has no transform!");
        ParentComponent* parent = mRegistry->GetComponent<ParentComponent>(mGoId);

        mat44 parentLW = kIdentityTag;
        if( parent )
        {
            GameObject parentGo(parent->parentGoId, *mRegistry);
            parentLW = parentGo.GetTransformLW();
        }

        return transform->localToParent * parentLW;
    }

    void GameObject::SetTransformLW(const mat44& m)
    {
        TransformComponent* transform = mRegistry->GetComponent<TransformComponent>(mGoId);
        SJ_ASSERT(transform, "GameObject has no transform!");
        
        ParentComponent* parent = mRegistry->GetComponent<ParentComponent>(mGoId);
        if(parent)
        {
            GameObject parentGo(parent->parentGoId, *mRegistry);
            mat44 worldToParent = parentGo.GetTransformLW().affine_inverse();
            transform->localToParent = m * worldToParent;
        }
        else
        {
            transform->localToParent = m;
        }
    }
}