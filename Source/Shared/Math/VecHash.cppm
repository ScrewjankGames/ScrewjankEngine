module;
// Adapted from https://github.com/g-truc/glm/blob/master/glm/gtx/hash.inl
// STD Headers
#include <functional>

export module sj.shared.math:VecHash;
import :Vec2;
import :Vec3;

export namespace sj
{
    inline void HashCombine(size_t& seed, size_t hash)
    {
        hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hash;
    }
}

export namespace std
{
    template <>
    struct hash<sj::Vec2>
    {
        size_t operator()(sj::Vec2 const& v) const
        {
            size_t seed = 0;
            hash<float> hasher;
            sj::HashCombine(seed, hasher(v.GetX()));
            sj::HashCombine(seed, hasher(v.GetY()));
            return seed;
        }
    };

    template <>
    struct hash<sj::Vec3>
    {
        size_t operator()(sj::Vec3 const& v) const
        {
            size_t seed = 0;
            hash<float> hasher;
            sj::HashCombine(seed, hasher(v[0]));
            sj::HashCombine(seed, hasher(v[1]));
            sj::HashCombine(seed, hasher(v[2]));
            return seed;
        }
    };

} // namespace std