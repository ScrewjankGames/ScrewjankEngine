#pragma once

namespace sj
{
    class Vec3
    {
    public:

        auto&& operator[](this auto&& self, int idx) // -> float& or const float&
        {
            return (&(self.x))[idx];
        }

        Vec3& operator+=(const Vec3& other);

        inline bool operator==(const Vec3& other) const 
        {
            return 
                x == other.x && 
                y == other.y &&
                z == other.z;
        }

        float x, y, z;
    };
} // namespace sj