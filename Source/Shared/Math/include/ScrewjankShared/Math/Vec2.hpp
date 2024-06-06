#pragma once
namespace sj
{
	class Vec2
	{
    public:
        Vec2() = default;
        Vec2(float x, float y);
        
        auto&& operator[](this auto&& self, int idx) // -> float& or const float&
        {
            return self.m_elements[idx];
        }

        [[nodiscard]] float GetX() const;
        [[nodiscard]] float GetY() const;

        [[nodiscard]] Vec2 operator*(float s) const;
        [[nodiscard]] Vec2 operator/(float s) const;


        static Vec2 Zero;
        static Vec2 UnitX;
        static Vec2 UnitY;

    private:
        float m_elements[2];
	};

    [[nodiscard]] float Magnitude(const Vec2& v);
    [[nodiscard]] float MagnitudeSqr(const Vec2& v);
    
    [[nodiscard]] Vec2 Normalized(const Vec2& v);
}