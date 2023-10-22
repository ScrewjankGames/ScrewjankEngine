#include <ScrewjankShared/Math/Vec3.hpp>

sj::Vec3::Vec3(float x, float y, float z) : m_elements {x, y, z}
{

}

float& sj::Vec3::operator[](int idx)
{
    return m_elements[idx];
}
