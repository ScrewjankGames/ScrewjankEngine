export module sj.std.math:Vec3;

export namespace sj
{
class vec3
{
public:
    auto&& operator[](this auto&& self, int idx) // -> float& or const float&
    {
        return (&(self.x))[idx];
    }

    vec3& operator+=(const vec3& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;

        return *this;
    }

    inline bool operator==(const vec3& other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }

    float x, y, z;
};
} // namespace sj