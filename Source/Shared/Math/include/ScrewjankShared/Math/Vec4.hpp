#pragma once

namespace sj
{
    // Forward Declares
    class Vec2;

    class alignas(16) Vec4
    {
    public:
        Vec4() = default;
        Vec4(const Vec2& v, float z = 0.0f, float w = 0.0f);
        Vec4(float x, float y, float z, float w);

        auto&& operator[](this auto&& self, int idx) // -> float& or const float&
        {
            return self.m_elements[idx];
        }

        [[nodiscard]] Vec4 operator*(const float s) const;
        [[nodiscard]] Vec4 operator/(const float s) const;
        [[nodiscard]] Vec4 operator-(const Vec4& other) const;

        [[nodiscard]] Vec4 operator-() const;

        [[nodiscard]] bool operator==(const Vec4& other) const;

        [[nodiscard]] float Dot(const Vec4& other) const;
        [[nodiscard]] float Magnitude() const;

        [[nodiscard]] Vec4 Cross(const Vec4& b) const;

        [[nodiscard]] bool Equals(const Vec4& other, float epsilon=0.0001f) const;

        float GetX() const;
        float GetY() const;
        float GetZ() const;
        float GetW() const;

        const static Vec4 UnitX;
        const static Vec4 UnitY;
        const static Vec4 UnitZ;
        const static Vec4 UnitW;

        const static Vec4 Right;
        const static Vec4 Up;
        const static Vec4 Forward;

    private:
        float m_elements[4] = {};
    };

    [[nodiscard]] Vec4 Normalize(const Vec4& v);
    [[nodiscard]] Vec4 Normalize3_W0(const Vec4& v);

}