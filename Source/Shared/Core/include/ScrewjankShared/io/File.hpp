#pragma once

// STD Headers
#include <cstddef>
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
            kWrite,
            kReadBinary,
            kWriteBinary
        };

        File() = default;
        ~File();

        bool Open(const char* path, OpenMode mode);
        void Close();

        /**
         * @return The number of bytes read 
         */
        size_t Read(void* buffer, size_t size, size_t count = 1);

        size_t Write(const void* buffer, size_t size, size_t count = 1);

        template <class T>
        size_t ReadStruct(T& dest) { return Read(&dest, sizeof(T)); }

        template<class T>
        size_t WriteStruct(const T& elem) { return Write(&elem, sizeof(T)); }

        [[nodiscard]] size_t Size();

        [[nodiscard]] bool IsOpen() const;

        [[nodiscard]] long CursorPos() const;

    private:
        FILE* m_File = nullptr;
    };
}
