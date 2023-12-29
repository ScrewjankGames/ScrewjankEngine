#include <ScrewjankShared/Math/Vec4.hpp>

namespace sj
{
    Vec4::Vec4(float x, float y, float z, float w) : m_elements{x, y, z, w}
    {
    }

    float& Vec4::operator[](int idx)
    {
        return m_elements[idx];
    }

    float& Vec4::X()
    {
        return m_elements[0];
    }

    float& Vec4::Y()
    {
        return m_elements[1];
    }

    float& Vec4::Z()
    {
        return m_elements[2];
    }

    float& Vec4::W()
    {
        return m_elements[3];
    }

} // namespace sj