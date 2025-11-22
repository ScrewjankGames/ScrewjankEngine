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

struct type_info
{
    std::string_view name = "";
    TypeId id = 0;

    size_t size = 0;
    size_t alignment = 0;

    bool is_trivially_destructible = false;

    /**
     * @param buffer: Location to construct new instance(s) of type
     * @param count: How many instances to construct in buffer
     */
    using ctorFn = void (*)(std::span<std::byte> buffer, size_t count);
    ctorFn constructor_fn = nullptr;

    /**
     * @param buffer: Location to destruct instance(s) of type
     * @param count: How many instances to destroy in buffer
     */
    using dtorFn = void (*)(std::span<std::byte> buffer, size_t count);
    dtorFn destructor_fn = nullptr;

    /**
     * @param oldBuffer: Element(s) to move-from
     * @param newBuffer: Uninitialized buffer to move to
     * @param count: How many instances to destroy in buffer
     */
    using moveFn = void (*)(std::span<std::byte> oldBuffer,
                            std::span<std::byte> newBuffer,
                            size_t count);
    moveFn move_constructor_fn = nullptr;
};

template <class T>
constexpr std::span<T> byte_span_cast(std::span<std::byte> buf, int expectedCount)
{
    auto name = glz::type_name<T>;

    SJ_ASSERT(buf.size() >= expectedCount * sizeof(T),
              "Buffer too small! Cannot address {} instances of {} in buffer of size {} bytes",
              expectedCount,
              name,
              buf.size());

    std::span<T> typedBuff {reinterpret_cast<T*>(buf.data()), buf.size() / sizeof(T)};
    return typedBuff;
}

template <class T>
constexpr TypeId type_id_of = string_hash(glz::type_name<T>).AsInt();

template <class T>
constexpr std::string_view type_name_of = glz::type_name<T>;

template <class T>
constexpr type_info type_info_of {
    .name = type_name_of<T>,
    .id = type_id_of<T>,
    .size = sizeof(T),
    .alignment = alignof(T),
    .is_trivially_destructible = std::is_trivially_destructible_v<T>,
    .constructor_fn =
        [](std::span<std::byte> buf, size_t count) {
            std::span<T> typedBuff = byte_span_cast<T>(buf, count);
            for(T& uninitialized : typedBuff)
            {
                new(&uninitialized) T();
            }
        },
    .destructor_fn =
        [](std::span<std::byte> buf, size_t count) {
            std::span<T> typedBuff = byte_span_cast<T>(buf, count);
            std::ranges::destroy(typedBuff);
        },
    .move_constructor_fn =
        [](std::span<std::byte> oldBuf, std::span<std::byte> newBuf, size_t count) {
            std::span<T> oldTypedBuf = byte_span_cast<T>(oldBuf, count);
            std::span<T> newTypedBuf = byte_span_cast<T>(oldBuf, count);

            std::ranges::uninitialized_move(oldTypedBuf, newTypedBuf);
        }};

} // namespace sj