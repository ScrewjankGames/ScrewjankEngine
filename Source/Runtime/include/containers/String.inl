// STD Includes
#include <string>

// Screwjank Includes
#include <containers/String.hpp>

namespace sj
{
    inline constexpr ConstString::ConstString(const char* str)
    {
        m_CStr = str;
    }

    inline bool operator==(const ConstString& left, const ConstString& right)
    {
        return strcmp(left.c_str(), right.c_str()) == 0;
    }

    inline bool operator!=(const ConstString& left, const ConstString& right)
    {
        return !(left == right);
    }

    inline std::strong_ordering operator<=>(const ConstString& left, const ConstString& right)
    {
        auto res = strcmp(left.c_str(), right.c_str());
        if (res < 0)
        {
            return std::strong_ordering::less;
        }
        else if (res == 0)
        {
            return std::strong_ordering::equal;
        }
        else
        {
            return std::strong_ordering::greater;
        }
    }

    inline const char* ConstString::c_str() const
    {
        return m_CStr;
    }

    inline ConstString::operator const char*() const
    {
        return m_CStr;
    }
}