module;

#include <ScrewjankStd/Assert.hpp>

module sj.engine.ecs.ECSRegistry;
import sj.std;
import sj.engine.TransformComponent;

namespace sj
{
void ECSRegistry::ReleaseGameObject(GameObjectId goId)
{
    GameObjectRecord* goRecord = mGameObjects.get<GameObjectRecord>(goId);
    SJ_ASSERT(goRecord, "Cannot release missing game object");

    Archetype* archetype = GetArchetype(goRecord->archetypeId);
    SJ_ASSERT(archetype, "Cannot release from null archetype for object ID");

    for(TypeId id : archetype->GetTypeIds())
    {
        auto callbackIt = mComponentDestroyCallbacks.find(id);
        if(callbackIt == mComponentDestroyCallbacks.end())
            continue;

        std::invoke(callbackIt->second,
                    goId,
                    archetype->GetEntry(id, goRecord->archetypeLocalIndex));
    }

    archetype->RemoveEntry(goRecord->archetypeLocalIndex);
}

Archetype* ECSRegistry::GetArchetype(ArchetypeId aId)
{
    auto archetypeIt = mArchetypes.find(aId);
    if(archetypeIt == mArchetypes.end())
        return nullptr;

    return archetypeIt->second.get();
}

mat44 GameObject::GetTransformLW() const
{
    TransformComponent* transform = mRegistry->GetComponent<TransformComponent>(mGoId);
    SJ_ASSERT(transform, "GameObject has no transform!");
    ParentComponent* parent = mRegistry->GetComponent<ParentComponent>(mGoId);

    mat44 parentLW = kIdentityTag;
    if(parent)
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
} // namespace sj