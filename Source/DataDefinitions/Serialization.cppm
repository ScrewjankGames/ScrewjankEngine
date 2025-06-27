module;

// SJ Includes
#include <ScrewjankStd/TypeMacros.hpp>

// Library Headers
#include <glaze/glaze.hpp>

export module sj.datadefs:Serialization;
import sj.std.math;

export namespace glz
{
    template <>
    struct from<JSON, sj::string_hash>
    {
        template <auto Opts>
        static void op(sj::string_hash& strHash, auto&&... args)
        {
            std::string str;
            glz::parse<JSON>::op<Opts>(str, args...);

            strHash = sj::string_hash(str);
        }
    };

    template <>
    struct to<BEVE, sj::string_hash>
    {
        template <auto Opts>
        static void op(sj::string_hash& hash, auto&&... args)
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
    struct from<JSON, sj::Vec3>
    {
        template <auto Opts>
        static void op(sj::Vec3& vec, auto&&... args)
        {
            std::array<float, 3> data = {};
            glz::parse<JSON>::op<Opts>(data, args...);
            vec = {.x=data[0], .y=data[1], .z=data[2]};
        }
    };

    template <>
    struct to<BEVE, sj::Vec3>
    {
        template <auto Opts>
        static void op(sj::Vec3& vec, auto&&... args)
        {
            std::array<float, 3> data = {vec.x, vec.y, vec.z};
            glz::serialize<BEVE>::op<Opts>(data, args...);
        }
    };

    template <>
    struct to<BEVE, sj::Vec4>
    {
        template <auto Opts>
        static void op(const sj::Vec4& vec, auto&&... args)
        {
            std::array<float, 4> data = vec.Data();
            glz::serialize<BEVE>::op<Opts>(data, args...);
        }
    };

    template <>
    struct from<BEVE, sj::Vec4>
    {
        template <auto Opts>
        static void op(sj::Vec4& vec, auto&&... args)
        {
            glz::parse<BEVE>::op<Opts>(vec.Data(), args...);
        }
    };

    template <>
    struct from<JSON, sj::Mat44>
    {
        template <auto Opts>
        static void op(sj::Mat44& transform, auto&&... args)
        {
            struct MatLayout
            {
                sj::Vec3 translation;
                sj::Vec3 rotation;
                sj::Vec3 scale;
            } layout;

            glz::parse<JSON>::op<Opts>(layout, args...);

            layout.rotation.x = sj::ToRadians(layout.rotation.x);
            layout.rotation.y = sj::ToRadians(layout.rotation.y);
            layout.rotation.z = sj::ToRadians(layout.rotation.z);

            transform = sj::BuildTransform(layout.scale, layout.rotation, sj::Vec4(layout.translation, 1));
        }
    };

    template <>
    struct to<BEVE, sj::Mat44>
    {
        template <auto Opts>
        static void op(const sj::Mat44& m, auto&&... args)
        {
            glz::serialize<BEVE>::op<Opts>(m.Data(), args...);
        }
    };

    template <>
    struct from<BEVE, sj::Mat44>
    {
        template <auto Opts>
        static void op(sj::Mat44& m, auto&&... args)
        {
            glz::parse<BEVE>::op<Opts>(m.Data(), args...);
        }
    };
}