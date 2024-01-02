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

    private:
        float m_elements[2];
	};
}