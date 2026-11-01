module;

#include <glaze/glaze.hpp>

#include <unordered_map>

export module sj.datadefs.Registration;

import sj.datadefs.ChunkWriter;

import sj.std.containers;
import sj.std.primitives;
import sj.std.rtti;
import sj.std.type_info;
import sj.datadefs.Serialization;

export namespace sj::datadefs
{
void RegisterEngineComponentTypes();

struct DatadefTypeInfo
{
    const type_info* typeInfo;

    using deserializeJsonFn = void (*)(typed_ptr dst, const glz::generic_u64& data);
    deserializeJsonFn deserializeJson = nullptr;

    using deserializeBeveFn = void (*)(typed_ptr dst, std::span<const u8> data);
    deserializeBeveFn deserializeBeve = nullptr;

    using serializeBeveSizeFn = int (*)(void);
    serializeBeveSizeFn serializeBeveSize = nullptr;
   
    using jsonToBeveFn = void (*)(const glz::generic_u64& src, ChunkWriter& dst);
    jsonToBeveFn jsonToBeve = nullptr;
};

template <class T>
constexpr DatadefTypeInfo DataDefTypeInfoOf {
    .typeInfo = &type_info_of<T>,
    .deserializeJson = LoadFromJson<T>,
    .deserializeBeve = LoadFromBeve<T>,
    .jsonToBeve = JsonToBeve<T>,
};

using TypeInfoLookup = std::unordered_map<TypeId, const DatadefTypeInfo*>;
TypeInfoLookup* GetLookup();

const DatadefTypeInfo* FindTypeInfo(TypeId id);

template <class T>
void RegisterComponentType()
{
    sj::rtti::register_type<T>();
    GetLookup()->emplace(type_id_of<T>, &DataDefTypeInfoOf<T>);
}
} // namespace sj::datadefs