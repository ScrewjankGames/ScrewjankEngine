module;

#include <bit>
#include <cstddef>
#include <cstdint>
#include <glaze/core/reflect.hpp>
#include <glaze/glaze.hpp>
#include <SDL3/SDL_gpu.h>

#include <tuple>
#include <type_traits>

export module sj.engine.rendering.VertexDescription;
import sj.std;

export namespace sj
{
template <class T>
constexpr SDL_GPUVertexElementFormat GetVertexElementFormat()
{
    switch(type_id_of<T>)
    {
        case type_id_of<Vec2>:
            return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        case type_id_of<Vec3>:
            return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
        case type_id_of<Vec4>:
            return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
    }

    return SDL_GPU_VERTEXELEMENTFORMAT_INVALID;
}

template <class T, u32 tSlot = 0>
inline constexpr SDL_GPUVertexBufferDescription VertexDescriptionOf =
    SDL_GPUVertexBufferDescription {
        .slot = tSlot,
        .pitch = sizeof(T),
        .input_rate = SDL_GPUVertexInputRate::SDL_GPU_VERTEXINPUTRATE_VERTEX,
        .instance_step_rate = 0,
    };

template <class VertexType>
    requires std::is_standard_layout_v<VertexType>
inline constexpr std::array VertexAttributesOf = []<class T>() {
    T dummy {};
    auto&& [... fields] = dummy;
    std::array<SDL_GPUVertexAttribute, sizeof...(fields)> attribs;

    u32 idx = 0;
    u32 byteOffset = 0; // track offset of members
    auto fieldToAttribFn = [&](const auto& field) {
        // acount for padding
        byteOffset += get_alignment_adjustment(alignof(decltype(field)), byteOffset);

        SDL_GPUVertexAttribute attr {
            .location = idx,
            .buffer_slot = 0,
            .format = GetVertexElementFormat<decltype(field)>(),
            .offset = byteOffset,
        };

        byteOffset += sizeof(field);

        return attr;
    };
    (void(attribs.at(idx++) = fieldToAttribFn(fields)), ...);

    return attribs;
}.template operator()<VertexType>();

template <class T>
inline constexpr SDL_GPUVertexInputState VertexInputState = SDL_GPUVertexInputState {
    .vertex_buffer_descriptions = &VertexDescriptionOf<T>,
    .num_vertex_buffers = 1,
    .vertex_attributes = VertexAttributesOf<T>.data(),
    .num_vertex_attributes = VertexAttributesOf<T>.size(),
};
} // namespace sj