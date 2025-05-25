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
        Quat(const Vec4& v) : m_elements {v[0], v[1], v[2], v[3]}
        {
        }

        auto&& operator[](this auto&& self, int idx) // -> float& or const float&
        {
            return self.m_elements[idx];
        }

    private:
        float m_elements[4] = {};
    };
} // namespace sj