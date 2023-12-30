#pragma once

namespace sj
{
    class alignas(16) Vec4
    {
    public:
        Vec4() = default;
        Vec4(float x, float y, float z, float w);

        auto&& operator[](this auto&& self, int idx) // -> float& or const float&
        {
            return self.m_elements[idx];
        }

        Vec4 operator*(const float s) const;
        Vec4 operator/(const float s) const;
        Vec4 operator-(const Vec4& other) const;

        Vec4 operator-() const;

        bool operator==(const Vec4& other) const;

        [[nodiscard]] float Dot(const Vec4& other) const;
        [[nodiscard]] float Magnitude() const;

        [[nodiscard]] Vec4 Cross(const Vec4& b) const;

        float& X();
        float& Y();
        float& Z();
        float& W();

        const static Vec4 UnitX;
        const static Vec4 UnitY;
        const static Vec4 UnitZ;
        const static Vec4 UnitW;

        const static Vec4 Right;
        const static Vec4 Up;
        const static Vec4 Forward;

    private:
        float m_elements[4];
    };

    [[nodiscard]] Vec4 Normalize(const Vec4& v);
    [[nodiscard]] Vec4 Normalize3_W0(const Vec4& v);

}