#include <ScrewjankShared/Math/Vec3.hpp>
#include <ScrewjankShared/Core/Assert.hpp>

sj::Vec3::Vec3(float x, float y, float z) : m_elements {x, y, z}
{

}

float& sj::Vec3::operator[](int idx)
{
    SJ_ASSERT()
    // TODO: insert return statement here
}
