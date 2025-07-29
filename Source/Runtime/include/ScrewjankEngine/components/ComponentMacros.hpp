#pragma once

#include <ScrewjankStd/TypeMacros.hpp>

#define SJ_COMPONENT(type, chunk_type)                                                             \
    SJ_STRUCT_TYPE_ID(type);                                                                       \
    static constexpr sj::TypeId kChunkTypeId = sj::string_hash(#chunk_type).AsInt();               \
    type(const DataChunk& chunk) : type(chunk.Get<chunk_type>())                                   \
    {                                                                                              \
    }
