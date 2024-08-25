#pragma once

namespace sj
{
    class Vec3
    {
    public:
        Vec3() = default;
        Vec3(float x, float y, float z);

        auto&& operator[](this auto&& self, int idx) // -> float& or const float&
        {
            return self.m_elements[idx];
        }

        Vec3& operator+=(const Vec3& other);

        inline bool operator==(const Vec3& other) const 
        {
            return 
                m_elements[0] == other.m_elements[0] && 
                m_elements[1] == other.m_elements[1] &&
                m_elements[2] == other.m_elements[2];
        }

    private:
        float m_elements[3];
    };
} // namespace sj