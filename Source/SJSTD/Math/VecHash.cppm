module;
// Adapted from https://github.com/g-truc/glm/blob/master/glm/gtx/hash.inl
// STD Headers
#include <functional>

export module sj.std.math:VecHash;
import :Vec2;
import :Vec3;

export namespace sj
{
inline void HashCombine(size_t& seed, size_t hash)
{
    hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hash;
}
} // namespace sj

export namespace std
{
template <>
struct hash<sj::vec2>
{
    size_t operator()(sj::vec2 const& v) const
    {
        size_t seed = 0;
        hash<float> hasher;
        sj::HashCombine(seed, hasher(v.get_x()));
        sj::HashCombine(seed, hasher(v.get_y()));
        return seed;
    }
};

template <>
struct hash<sj::vec3>
{
    size_t operator()(sj::vec3 const& v) const
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