module;

// Screwjank Headers
#include <ScrewjankStd/Assert.hpp>

// Library Headers
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
    glz::generic data = glz::generic::object_t{};

    // Unpacks byte buffer into requested type
    template <class T>
    [[nodiscard]] auto Get() const -> T
    {
        T val;
        glz::error_ctx err = glz::read<glz::opts {}>(val, data);
        SJ_ASSERT(err == glz::error_code::none, "Failed to load data chunk!");

        return val;
    }

    void UnknownRead(const glz::sv& key, const glz::raw_json& value)
    {
        glz::generic::object_t& obj = data.get_object();
        obj[std::string(key)] = glz::read_json<glz::generic>(value.str).value_or({});
    }
};
} // namespace sj

export template <>
struct glz::meta<sj::DataChunk>
{
    using T = sj::DataChunk;
    static constexpr auto value = object("type", &T::type);
    static constexpr auto unknown_read {&T::UnknownRead};
};