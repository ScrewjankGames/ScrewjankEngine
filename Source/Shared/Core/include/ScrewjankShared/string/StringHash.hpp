#pragma once

// STD Includes
#include <cstdint>
#include <compare>
#include <string_view>

namespace sj
{
    using TypeId = uint32_t;

    constexpr uint32_t FNV1a_32(std::string_view str, uint32_t seed = 0x811c9dc5)
    {
        for(char c : str)
        {
            seed = seed ^ c;
            seed = seed * 0x01000193;
        }

        return seed;
    }

    class StringHash
    {
    public:

        inline constexpr StringHash() : m_hash(0) { }

        inline constexpr StringHash(const char* str) : m_hash(FNV1a_32(str))
        {

        }

        inline constexpr StringHash(std::string_view str) : m_hash(FNV1a_32(str))
        {

        }

        inline constexpr std::strong_ordering operator<=>(const StringHash& other) const { return m_hash <=> other.m_hash; }
        inline constexpr bool operator==(const StringHash& other) const { return m_hash == other.m_hash; }
        inline constexpr bool operator!=(const StringHash& other) const { return !(*this == other); }

        inline constexpr uint32_t AsInt() const { return m_hash; }

    private:
        uint32_t m_hash;
    };

    inline constexpr sj::StringHash operator""_strhash(const char* str, std::size_t)
    {
        return sj::StringHash(str);
    }
}

#define SJ_STRUCT_TYPE_ID(type)                                                                    \
    static constexpr sj::TypeId kTypeId = sj::StringHash(#type).AsInt();

namespace std
{
    template <>
    struct hash<sj::StringHash>
    {
        std::size_t operator()(const sj::StringHash& strHash) const
        {
            return std::hash<uint32_t>()(strHash.AsInt());
        }
    };
} // namespace std
