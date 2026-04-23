module;

#include <cstdint>
#include <span>
#include <ranges>
#include <string_view>

export module sj.std.hash;

export namespace sj
{
inline constexpr uint32_t fnv_offset_basis_32 = 0x811c9dc5;
inline constexpr uint32_t fnv_prime_32 = 0x01000193;

constexpr uint32_t FNV1a_32(std::span<std::byte> bytes, uint32_t hash = fnv_offset_basis_32)
{
    for(std::byte b : bytes)
    {
        hash = hash ^ static_cast<char>(b);
        hash = hash * fnv_prime_32;
    }

    return hash;
}

constexpr uint32_t FNV1a_32(std::string_view str, uint32_t hash = fnv_offset_basis_32)
{
    for(char c : str)
    {
        hash = hash ^ c;
        hash = hash * fnv_prime_32;
    }

    return hash;
}

constexpr uint32_t FNV1a_32(std::ranges::range auto elems, uint32_t hash = fnv_offset_basis_32)
{
    for(auto&& elem : elems)
    {
        std::span bytes = std::as_bytes(std::span(&elem, 1));
        hash = FNV1a_32(bytes, hash);
    }
    return hash;
}

inline constexpr uint64_t fnv_offset_basis_64 = 0xcbf29ce484222325;
inline constexpr uint64_t fnv_prime_64 = 0x00000100000001b3;
constexpr uint64_t FNV1a_64(std::span<std::byte> bytes, uint64_t hash = fnv_offset_basis_64)
{
    for(std::byte b : bytes)
    {
        hash = hash ^ static_cast<char>(b);
        hash = hash * fnv_prime_64;
    }

    return hash;
}

constexpr uint64_t FNV1a_64(std::string_view str, uint64_t hash = fnv_offset_basis_64)
{
    for(char c : str)
    {
        hash = hash ^ c;
        hash = hash * fnv_prime_64;
    }

    return hash;
}

constexpr uint64_t FNV1a_64(std::ranges::range auto elems, uint64_t hash = fnv_offset_basis_64)
{
    for(auto&& elem : elems)
    {
        std::span bytes = std::as_bytes(std::span(&elem, 1));
        hash = FNV1a_64(bytes, hash);
    }
    return hash;
}
} // namespace sj