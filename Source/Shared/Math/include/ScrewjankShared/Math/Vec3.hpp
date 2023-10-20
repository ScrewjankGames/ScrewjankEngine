#pragma once

namespace sj
{
    class Vec3
    {
    public:
        Vec3() = default;
        Vec3(float x, float y, float z);

        float& operator[](int idx);

    private:
        float m_elements[3];
    };
} // namespace sj