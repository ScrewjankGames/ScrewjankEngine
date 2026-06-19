module;
#include <cstddef>
#include <string_view>
#include <span>
#include <type_traits>

#include <glaze/glaze.hpp>

#include <ScrewjankStd/Assert.hpp>
export module sj.std.type_info;
import sj.std.string_hash;

export namespace sj
{
using TypeId = uint32_t;

template <class T>
constexpr TypeId type_id_of = string_hash(glz::type_name<T>).AsInt();

template <class T>
constexpr std::string_view type_name_of = glz::type_name<T>;

struct type_info
{
    std::string_view name = "";
    TypeId id = 0;

    size_t size = 0;
    size_t alignment = 0;

    bool is_trivially_destructible = false;

    using ctorFn = void (*)(void* addr);
    ctorFn constructor_fn = nullptr;

    using dtorFn = void (*)(void* addr);
    dtorFn destructor_fn = nullptr;

    /**
     * @param oldBuffer: Element(s) to move-from
     * @param newBuffer: Uninitialized buffer to move to
     */
    using moveFn = void (*)(void* oldAddr, void* newAddr);
    moveFn move_constructor_fn = nullptr;

    using deserializeJsonFn = void (*)(void* dst, const glz::generic_u64& data);
    deserializeJsonFn desierialize_json_fn = nullptr;
};

template <class T>
constexpr std::span<T> byte_span_cast(std::span<std::byte> buf)
{
    auto name = glz::type_name<T>;

    SJ_ASSERT(
        buf.size() % sizeof(T) == 0,
        "Buffer not correct size! Buffer does not divide evenly by contained type {}'s size {}",
        name,
        buf.size());

    std::span<T> typedBuff {reinterpret_cast<T*>(buf.data()), buf.size() / sizeof(T)};
    return typedBuff;
}

template <class T>
constexpr type_info type_info_of {
    .name = type_name_of<T>,
    .id = type_id_of<T>,
    .size = sizeof(T),
    .alignment = alignof(T),
    .is_trivially_destructible = std::is_trivially_destructible_v<T>,
    .constructor_fn =
        [](void* addr) {
            std::construct_at<T>(reinterpret_cast<T*>(addr));
        },
    .destructor_fn =
        [](void* addr) {
            std::destroy_at<T>(reinterpret_cast<T*>(addr));
        },
    .move_constructor_fn =
        [](void* newAddr, void* oldAddr) {
            new(newAddr) T(std::move(*reinterpret_cast<T*>(oldAddr)));
        },
    .desierialize_json_fn =
        [](void* dst, const glz::generic_u64& data) {
            glz::error_ctx err = glz::read<glz::opts {}>(*reinterpret_cast<T*>(dst), data);
            SJ_ASSERT(err == glz::error_code::none,
                      "Failed to desierialize json data for type {}! Reason: {}",
                      type_name_of<T>,
                      glz::format_error(err));
        },
};

} // namespace sj