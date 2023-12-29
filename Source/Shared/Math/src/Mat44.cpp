#include <ScrewjankShared/Math/Mat44.hpp>

namespace sj
{
	Mat44::Mat44(Vec4 x, Vec4 y, Vec4 z, Vec4 w) 
		: m_rows {x, y, z, w}
	{
    }

    Vec4& Mat44::operator[](int row)
    {
        return m_rows[row];
    }

    Vec4& Mat44::X()
    {
        return m_rows[0];
    }

    Vec4& Mat44::Y()
    {
        return m_rows[1];
    }

    Vec4& Mat44::Z()
    {
        return m_rows[2];
    }

    Vec4& Mat44::W()
    {
        return m_rows[3];
    }
}