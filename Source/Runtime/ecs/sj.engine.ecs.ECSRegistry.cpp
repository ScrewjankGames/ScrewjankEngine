module;

#include <ScrewjankStd/Assert.hpp>
#include <ranges>

module sj.engine.ecs.ECSRegistry;
import sj.std;
import sj.engine.system.threading;
import sj.engine.TransformComponent;

namespace sj
{
GameObjectId ECSRegistry::CreateGameObject(std::span<const TypeId> typeIds,
                                           std::span<const std::span<const u8>> chunks,
                                           std::optional<GameObjectId> parent)
{
    auto scratchpad = ThreadContext::GetScratchpad();

    dynamic_vector<const sj::type_info*> typeInfos(&scratchpad);
    typeInfos.reserve(typeIds.size() + (parent.has_value() ? 1 : 0));

    for(TypeId id : typeIds)
    {
        const type_info* info = rtti::find_type_info(id);
        SJ_ASSERT(info, "Failed to find type info for type id {}", id);
        typeInfos.emplace_back(info);
    }

    if(parent)
        typeInfos.emplace_back(&type_info_of<ParentComponent>);

    ArchetypeId aId = ComputeArchetypeId(typeInfos);
    GameObjectId newGoId = mGameObjects.create(GameObjectRecord {.archetypeId = aId});

    Archetype* archetype = FindOrAddArchetype(aId, typeInfos);
    uZ row = archetype->AddEntryUninitialized(newGoId);

    std::span serializedChunkInfos =
        parent ? std::span(typeInfos).subspan(0, typeInfos.size() - 1) : std::span(typeInfos);
    for(auto&& [typeInfo, chunk] : std::views::zip(serializedChunkInfos, chunks))
    {
        typed_ptr componentDst = archetype->GetEntry(typeInfo->id, row);
        typeInfo->desierialize_beve_fn(componentDst.get_ptr(), chunk);
    }

    if(parent)
    {
        typed_ptr parentComponentDst = archetype->GetEntry(type_info_of<ParentComponent>.id, row);
        new(parentComponentDst.as<ParentComponent>()) ParentComponent(parent.value());
    }

    return newGoId;
}

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

mat44 GameObject::GetTransformLS() const
{
    TransformComponent* transform = mRegistry->GetComponent<TransformComponent>(mGoId);
    SJ_ASSERT(transform, "GameObject has no transform!");
    return transform->localToParent;
}

void GameObject::SetTransformLS(const mat44& m)
{
    TransformComponent* transform = mRegistry->GetComponent<TransformComponent>(mGoId);
    SJ_ASSERT(transform, "GameObject has no transform!");
    transform->localToParent = m;
}

mat44 GameObject::GetTransformLW() const
{
    ParentComponent* parent = mRegistry->GetComponent<ParentComponent>(mGoId);

    mat44 parentLW = kIdentityTag;
    if(parent)
    {
        GameObject parentGo(parent->parentGoId, *mRegistry);
        parentLW = parentGo.GetTransformLW();
    }

    return GetTransformLS() * parentLW;
}

void GameObject::SetTransformLW(const mat44& m)
{
    ParentComponent* parent = mRegistry->GetComponent<ParentComponent>(mGoId);
    if(parent)
    {
        GameObject parentGo(parent->parentGoId, *mRegistry);
        mat44 worldToParent = parentGo.GetTransformLW().affine_inverse();
        SetTransformLS(m * worldToParent);
    }
    else
    {
        SetTransformLS(m);
    }
}
} // namespace sj