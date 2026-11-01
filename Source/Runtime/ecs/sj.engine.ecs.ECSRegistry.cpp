module;

#include <ScrewjankStd/Assert.hpp>
#include <ranges>

module sj.engine.ecs.ECSRegistry;
import sj.engine.ecs.ParentComponent;
import sj.engine.system.threading;

import sj.datadefs.Registration;
import sj.datadefs.components.TransformComponent;

import sj.std;

namespace sj
{

GameObjectId ECSRegistry::CreateGameObject(std::span<const TypeId> typeIds,
                                           std::span<const std::span<const u8>> chunks,
                                           std::optional<GameObjectId> parent)
{
    auto scratchpad = ThreadContext::GetScratchpad();

    dynamic_vector<const datadefs::DatadefTypeInfo*> datadefTypeInfos(&scratchpad);
    datadefTypeInfos.reserve(typeIds.size() + (parent.has_value() ? 1 : 0));

    for(TypeId id : typeIds)
    {
        const datadefs::DatadefTypeInfo* info = datadefs::FindTypeInfo(id);
        SJ_ASSERT(info, "Failed to find type info for type id {}", id);
        datadefTypeInfos.emplace_back(info);
    }

    if(parent)
        datadefTypeInfos.emplace_back(&datadefs::DataDefTypeInfoOf<ParentComponent>);

    std::ranges::range auto typeInfos =
        datadefTypeInfos | std::views::transform(&datadefs::DatadefTypeInfo::typeInfo);

    ArchetypeId aId = ComputeArchetypeId(typeInfos);
    GameObjectId newGoId = mGameObjects.create(GameObjectRecord {.archetypeId = aId});
    GameObjectRecord* newRecord = mGameObjects.get<GameObjectRecord>(newGoId);

    Archetype* archetype = FindOrAddArchetype(aId, typeInfos);

    uZ row = archetype->AddEntryUninitialized(newGoId);

    newRecord->archetypeLocalIndex = row;

    std::span serializedChunkInfos =
        parent ? std::span(datadefTypeInfos).subspan(0, datadefTypeInfos.size() - 1)
               : std::span(datadefTypeInfos);
    for(auto&& [datadefInfo, chunk] : std::views::zip(serializedChunkInfos, chunks))
    {
        typed_ptr componentDst = archetype->GetEntry(datadefInfo->typeInfo->id, row);
        datadefInfo->deserializeBeve(componentDst, chunk);
    }

    if(parent)
    {
        typed_ptr parentComponentDst = archetype->GetEntry(type_info_of<ParentComponent>.id, row);
        new(parentComponentDst.as<ParentComponent>()) ParentComponent(parent.value());
    }

    for(const type_info* info : typeInfos)
    {
        auto callbackIt = mComponentCreateCallbacks.find(info->id);
        if(callbackIt == mComponentCreateCallbacks.end())
            continue;

        std::invoke(callbackIt->second, newGoId, archetype->GetEntry(info->id, row));
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