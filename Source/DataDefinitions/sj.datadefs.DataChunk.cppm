module;

// Screwjank Headers
#include <ScrewjankStd/Assert.hpp>

// Library Headers
#include <glaze/core/context.hpp>
#include <glaze/core/reflect.hpp>
#include <glaze/glaze.hpp>

// STD Headers

export module sj.datadefs.DataChunk;
import sj.datadefs.Serialization;
import sj.std.type_info;
import sj.std.string_hash;

export namespace sj
{
struct DataChunk
{
    hashed_string_sv type;
    glz::generic_u64 data = glz::generic_u64::object_t {};

    // Unpacks byte buffer into requested type
    template <class T>
    [[nodiscard]] auto Get() const -> T
    {
        T val;

        type_info_of<T>.desierialize_json_fn(&val, data);

        return val;
    }

    void UnknownRead(const glz::sv& key, const glz::raw_json& value)
    {
        glz::generic_u64::object_t& obj = data.get_object();
        obj[std::string(key)] = glz::read_json<glz::generic_u64>(value.str).value_or({});
    }
};
} // namespace sj

template <>
struct glz::meta<sj::DataChunk>
{
    using T = sj::DataChunk;
    static constexpr auto value = object("type", &T::type);
    static constexpr auto unknown_read {&T::UnknownRead};
};