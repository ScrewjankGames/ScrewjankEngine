module;

#include <array>

export module sj.shared.math:Quat;
import :Vec4;
import :Tags;

export namespace sj
{
    class alignas(16) Quat
    {
    public:
        Quat() = default;
        Quat(IdentityTagT _) : m_elements {0.0f, 0.0f, 0.0f, 1.0f}
        {
        }
        Quat(float x, float y, float z, float w) : m_elements {x, y, z, w}
        {
        }
        Quat(const Vec4& v) : m_elements( v.Data() )
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