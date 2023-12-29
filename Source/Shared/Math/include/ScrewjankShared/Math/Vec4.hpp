#pragma once

namespace sj
{
    class Vec4
    {
    public:
        Vec4() = default;
        Vec4(float x, float y, float z, float w);

        float& operator[](int idx);

        float& X();
        float& Y();
        float& Z();
        float& W();

    private:
        float m_elements[4];
    };
}