#pragma once

namespace sj
{
	class Vec2
	{
    public:
        Vec2() = default;
        Vec2(float x, float y);

        float& operator[](int idx);

    private:
        float m_elements[2];
	};
}