module;

#include <glaze/glaze.hpp>

#include <string_view>

export module sj.datadefs.Serialization;
import sj.std.string_hash;
import sj.std.math;

export namespace glz
{

template <>
struct meta<sj::string_hash>
{
    using mimic = std::string;
};

template <>
struct from<JSON, sj::string_hash>
{
    template <auto Opts>
    static void op(sj::string_hash& strHash, glz::is_context auto&& ctx, auto&& it, auto&& end)
    {
        std::string_view str;
        glz::parse<JSON>::op<Opts>(str, ctx, it, end);

        strHash = sj::string_hash(str);
    }
};

template <>
struct to<JSON, sj::string_hash>
{
    template <auto Opts>
    static void op(const sj::string_hash& hash, auto&&... args)
    {
        glz::serialize<JSON>::op<Opts>(hash.AsInt(), args...);
    }
};

template <>
struct meta<sj::hashed_string_sv>
{
    using mimic = std::string;
};

template <>
struct from<JSON, sj::hashed_string_sv>
{
    template <auto Opts>
    static void op(sj::hashed_string_sv& strHash, glz::is_context auto&& ctx, auto&& it, auto&& end)
    {
        std::string_view str;
        glz::parse<JSON>::op<Opts>(str, ctx, it, end);

        strHash = sj::hashed_string_sv(str);
    }
};

template <>
struct to<JSON, sj::hashed_string_sv>
{
    template <auto Opts>
    static void op(const sj::hashed_string_sv& hashedStr, auto&&... args)
    {
        glz::serialize<JSON>::op<Opts>(hashedStr.get_string(), args...);
    }
};

template <>
struct to<BEVE, sj::string_hash>
{
    template <auto Opts>
    static void op(const sj::string_hash& hash, auto&&... args)
    {
        glz::serialize<BEVE>::op<Opts>(hash.AsInt(), args...);
    }
};

template <>
struct from<BEVE, sj::string_hash>
{
    template <auto Opts>
    static void op(sj::string_hash& strHash, auto&&... args)
    {
        uint32_t hashVal = 0;
        glz::parse<BEVE>::op<Opts>(hashVal, args...);

        strHash = sj::string_hash(hashVal);
    }
};

template <>
struct from<JSON, sj::vec4>
{
    template <auto Opts>
    static void op(sj::vec4& vec, auto&&... args)
    {
        std::array<float, 4> data = {};
        glz::parse<JSON>::op<Opts>(data, args...);
        vec = sj::vec4(data[0], data[1], data[2], data[3]);
    }
};

template <>
struct from<JSON, sj::vec3>
{
    template <auto Opts>
    static void op(sj::vec3& vec, auto&&... args)
    {
        std::array<float, 3> data = {};
        glz::parse<JSON>::op<Opts>(data, args...);
        vec = {.x = data[0], .y = data[1], .z = data[2]};
    }
};

template <>
struct from<JSON, sj::vec2>
{
    template <auto Opts>
    static void op(sj::vec2& vec, auto&&... args)
    {
        std::array<float, 2> data = {};
        glz::parse<JSON>::op<Opts>(data, args...);
        vec = sj::vec2(data[0], data[1]);
    }
};

template <>
struct to<BEVE, sj::vec3>
{
    template <auto Opts>
    static void op(const sj::vec3& vec, auto&&... args)
    {
        std::array<float, 3> data = {vec.x, vec.y, vec.z};
        glz::serialize<BEVE>::op<Opts>(data, args...);
    }
};

template <>
struct to<BEVE, sj::vec4>
{
    template <auto Opts>
    static void op(const sj::vec4& vec, auto&&... args)
    {
        std::array<float, 4> data = vec.data();
        glz::serialize<BEVE>::op<Opts>(data, args...);
    }
};

template <>
struct from<BEVE, sj::vec4>
{
    template <auto Opts>
    static void op(sj::vec4& vec, auto&&... args)
    {
        glz::parse<BEVE>::op<Opts>(vec.data(), args...);
    }
};

template <>
struct from<JSON, sj::mat44>
{
    template <auto Opts>
    static void op(sj::mat44& transform, auto&&... args)
    {
        struct MatLayout
        {
            sj::vec3 translation;
            sj::vec3 rotation;
            sj::vec3 scale;
        } layout;

        glz::parse<JSON>::op<Opts>(layout, args...);

        layout.rotation.x = sj::to_rads(layout.rotation.x);
        layout.rotation.y = sj::to_rads(layout.rotation.y);
        layout.rotation.z = sj::to_rads(layout.rotation.z);

        transform =
            sj::build_transform(layout.scale, layout.rotation, sj::vec4(layout.translation, 1));
    }
};

template <>
struct to<BEVE, sj::mat44>
{
    template <auto Opts>
    static void op(const sj::mat44& m, auto&&... args)
    {
        glz::serialize<BEVE>::op<Opts>(m.data(), args...);
    }
};

template <>
struct from<BEVE, sj::mat44>
{
    template <auto Opts>
    static void op(sj::mat44& m, auto&&... args)
    {
        glz::parse<BEVE>::op<Opts>(m.data(), args...);
    }
};
} // namespace glz