#pragma once

// STD Headers
#include <cstddef>

// Library Headers

// Screwjank Headers

namespace sj {
   
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