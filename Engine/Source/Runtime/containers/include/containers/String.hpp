#pragma once

// STD Headers

// Library Headers

// Screwjank Headers

namespace sj {

    constexpr size_t FNV1aSeed = 0x811c9dc5;

    inline constexpr size_t FNV1aHash(const char* input, const uint32_t value = FNV1aSeed) noexcept
    {
        constexpr size_t prime = 0x1000193;

        if (input[0] == '\0')
        {
            return value;
        }
        else
        {
            return FNV1aHash(&input[1], (value ^ input[0]) * prime);
        }
    }

    /**
     * C String wrapper for immutable strings, with conversion and comparison operators  
     */
    class ConstString
    {
      public:
        /**
         * Constructor
         */
        constexpr ConstString(const char* str);

        /**
         * Destructor  
         */
        ~ConstString() = default;

        /**
         * Disallow assignment  
         */
        bool operator=(const char* other) = delete;

        /**
         * @return C-style string  
         */
        const char* c_str() const;

        /**
         * Implicit C-String Conversion operator  
         */
        explicit operator const char*() const;

      private:
        /** Underlying C string */
        const char* m_CStr;
    };

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

} // namespace sj

namespace std
{
    template <>
    struct hash<sj::ConstString>
    {
        std::size_t operator()(const sj::ConstString& value) const
        {
            return sj::FNV1aHash(value.c_str());
        }
    };

} // namespace std