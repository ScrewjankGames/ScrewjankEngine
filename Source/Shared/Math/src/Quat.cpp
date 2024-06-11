// Parent Include
#include <ScrewjankShared/Math/Quat.hpp>

// Shared Includes
#include <ScrewjankShared/Math/Vec4.hpp>

// STD Includes

namespace sj
{
    Quat::Quat(IdentityTagT) : m_elements {0.0f, 0.0f, 0.0f, 1.0f}
    {

    }

    Quat::Quat(float x, float y, float z, float w) : m_elements {x, y, z, w}
    {

    }

    Quat::Quat(const Vec4& v) : m_elements {v[0], v[1], v[2], v[3]}
    {
    }
} // namespace sj
