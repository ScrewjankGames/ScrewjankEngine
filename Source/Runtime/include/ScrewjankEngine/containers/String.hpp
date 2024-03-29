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
    
    // Jesus fuck why is strncpy's default behavior not to null terminate on overrun
    inline char* sj_strncpy(char* destination, const char* source, size_t count)
    {
        for (int i = 0; i < count; i++)
        {
            if (source[i] == '\0')
            {
                break;
            }

            destination[i] = source[i];
        }

        // Always force null termination of string.
        destination[count-1] = '\0';
        return destination;
    }

} // namespace sj

// Include inlines
#include <ScrewjankEngine/containers/String.inl>