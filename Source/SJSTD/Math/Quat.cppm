module;

#include <array>

export module sj.std.math:Quat;
import :Vec4;
import :Tags;

export namespace sj
{
class alignas(16) quat
{
public:
    quat() = default;
    quat(IdentityTagT _) : m_elements {0.0f, 0.0f, 0.0f, 1.0f}
    {
    }
    quat(float x, float y, float z, float w) : m_elements {x, y, z, w}
    {
    }
    quat(const vec4& v) : m_elements(v.data())
    {
    }

    auto&& operator[](this auto&& self, int idx) // -> float& or const float&
    {
        return self.m_elements[idx];
    }

private:
    std::array<float, 4> m_elements;
};
} // namespace sj