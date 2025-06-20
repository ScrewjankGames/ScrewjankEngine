module;

// STD Includes
#include <cstdint>
#include <compare>
#include <string_view>

export module sj.std.string_hash;

export namespace sj
{
    constexpr uint32_t FNV1a_32(std::string_view str, uint32_t seed = 0x811c9dc5)
    {
        for(char c : str)
        {
            seed = seed ^ uint32_t(c);
            seed = seed * 0x01000193;
        }

        return seed;
    }

    class string_hash
    {
    public:

        inline constexpr string_hash() : m_hash(0) { }

        inline explicit constexpr string_hash(uint32_t intVal) : m_hash(intVal)
        {
            
        }

        inline constexpr string_hash(const char* str) : m_hash(FNV1a_32(str))
        {

        }

        inline constexpr string_hash(std::string_view str) : m_hash(FNV1a_32(str))
        {

        }

        inline constexpr std::strong_ordering operator<=>(const string_hash& other) const { return m_hash <=> other.m_hash; }
        inline constexpr bool operator==(const string_hash& other) const { return m_hash == other.m_hash; }
        inline constexpr bool operator!=(const string_hash& other) const { return !(*this == other); }

        inline constexpr uint32_t AsInt() const { return m_hash; }

    private:
        uint32_t m_hash;
    };

    inline constexpr sj::string_hash operator""_strhash(const char* str, std::size_t)
    {
        return sj::string_hash(str);
    }
}

namespace std
{
    template <>
    struct hash<sj::string_hash>
    {
        std::size_t operator()(const sj::string_hash& strHash) const
        {
            return std::hash<uint32_t>()(strHash.AsInt());
        }
    };
} // namespace std
