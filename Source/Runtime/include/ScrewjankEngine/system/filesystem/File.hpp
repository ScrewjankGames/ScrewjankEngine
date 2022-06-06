#pragma once

// STD Headers
#include <cstdio>
#include <stdint.h>

namespace sj
{
    class File
    {
    public:
        enum class OpenMode : uint8_t
        {
            kRead,
            kWrite
        };

        bool Open(const char* path, OpenMode mode);
        void Close();

        /**
         * @return The number of bytes read 
         */
        size_t Read(void* buffer, size_t size, size_t count = 1);

        uint64_t Size();

        bool IsOpen() const;

    private:
        FILE* m_File = NULL;
    };
}