module;

// Shared Includes
#include <ScrewjankShared/string/StringHash.hpp>

// Library Headers
#include <glaze/glaze.hpp>

export module sj.shared.datadefs:Serialization;
import sj.shared.math;

export namespace glz
{
    template <>
    struct from<JSON, sj::StringHash>
    {
        template <auto Opts>
        static void op(sj::StringHash& strHash, auto&&... args)
        {
            std::string str;
            glz::parse<JSON>::op<Opts>(str, args...);

            strHash = sj::StringHash(str);
        }
    };

    template <>
    struct to<BEVE, sj::StringHash>
    {
        template <auto Opts>
        static void op(sj::StringHash& hash, auto&&... args)
        {
            glz::serialize<BEVE>::op<Opts>(hash.AsInt(), args...);
        }
    };
    
    template <>
    struct from<BEVE, sj::StringHash>
    {
        template <auto Opts>
        static void op(sj::StringHash& strHash, auto&&... args)
        {
            uint32_t hashVal = 0;
            glz::parse<BEVE>::op<Opts>(hashVal, args...);

            strHash = sj::StringHash(hashVal);
        }
    };

    template <>
    struct from<JSON, sj::Vec3>
    {
        template <auto Opts>
        static void op(sj::Vec3& vec, auto&&... args)
        {
            std::array<float, 3> data;
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

            transform = sj::BuildTransform(layout.scale, layout.rotation, layout.translation);
        }
    };

    template <>
    struct to<BEVE, sj::Mat44>
    {
        template <auto Opts>
        static void op(sj::Mat44& m, auto&&... args)
        {
            std::array<float, 16> data = 
            {
                m[0][0], m[0][1], m[0][2], m[0][3],
                m[1][0], m[1][1], m[1][2], m[1][3],
                m[2][0], m[2][1], m[2][2], m[2][3],
                m[3][0], m[3][1], m[3][2], m[3][3]
            };

            glz::serialize<BEVE>::op<Opts>(data, args...);
        }
    };

    template <>
    struct from<BEVE, sj::Mat44>
    {
        template <auto Opts>
        static void op(sj::Mat44& m, auto&&... args)
        {
            std::array<float, 16> data;
            glz::parse<BEVE>::op<Opts>(data, args...);

            sj::Vec4 x (data[0], data[1], data[2], data[3]);
            sj::Vec4 y (data[4], data[5], data[6], data[7]);
            sj::Vec4 z (data[8], data[9], data[10], data[11]);
            sj::Vec4 w (data[12], data[13], data[14], data[15]);

            m = {x, y, z, w};
        }
    };
}