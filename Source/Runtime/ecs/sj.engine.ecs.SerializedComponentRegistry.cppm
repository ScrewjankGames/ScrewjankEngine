module;
#include <flat_map>
#include <optional>
export module sj.engine.ecs.SerializedComponentRegistry;

import sj.std.type_info;

export namespace sj
{
using LoadChunkLookup = std::flat_map<TypeId, TypeId>;
LoadChunkLookup* GetLoadChunkLookupTable()
{
    static LoadChunkLookup sLoadChunkLookup;
    return &sLoadChunkLookup;
}

const std::optional<TypeId> FindComponentType(TypeId loadChunkType)
{
    auto it = GetLoadChunkLookupTable()->find(loadChunkType);
    if(it == GetLoadChunkLookupTable()->end())
        return {};

    return it->second;
}

template <class T>
void RegisterComponentLoadChunk()
{
    TypeId loadChunkId = type_id_of<typename T::LoadChunkType>;

    auto res = GetLoadChunkLookupTable()->try_emplace(loadChunkId, type_id_of<T>);
}

} // namespace sj