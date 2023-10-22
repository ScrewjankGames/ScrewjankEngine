#include <ScrewjankShared/Math/Vec2.hpp>

sj::Vec2::Vec2(float x, float y) : m_elements {x, y}
{

}

float& sj::Vec2::operator[](int idx)
{
    return m_elements[idx];
}
