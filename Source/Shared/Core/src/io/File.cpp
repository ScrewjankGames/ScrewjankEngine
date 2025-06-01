// Parent
#include <ScrewjankShared/io/File.hpp>

// Screwjank Headers
#include <ScrewjankStd/Assert.hpp>

// STD Headers
#include <cstdio>

namespace sj
{
    File::~File()
    {
        Close();
    }

    bool File::Open(const char* path, OpenMode mode)
    {
        SJ_ASSERT(m_File == nullptr, "Close file before opening something else!");

        const char* modeString = [&mode]() 
        {
            switch(mode)
            {
            case File::OpenMode::kRead:
                return "r";
            case File::OpenMode::kWrite:
                return "w";
            case File::OpenMode::kReadBinary:
                return "rb";
            case File::OpenMode::kWriteBinary:
                return "wb";
            default:
                return "";
                break;
            }
        }();

        m_File = std::fopen(path, modeString);
       
        return m_File != nullptr;
    }

    void File::Close()
    {
        if(m_File)
        {
            fclose(m_File);
            m_File = nullptr;
        }
    }

    size_t File::Read(void* buffer, size_t size, size_t count)
    {
        size_t blockCount = fread(buffer, size, count, m_File);
        return blockCount * size;
    }

    size_t File::Write(const void* buffer, size_t size, size_t count)
    {
        size_t blockCount = fwrite(buffer, size, count, m_File);
        return blockCount * size;
    }

    uint64_t File::Size()
    {
        if (m_File == nullptr)
        {
            return 0;
        }

        // Save off cursor position
        long cursorPos = ftell(m_File);

        // Seek to end of file
        fseek(m_File, 0, SEEK_END);
        uint64_t size = ftell(m_File);

        // Restore cursor position
        fseek(m_File, cursorPos, SEEK_SET);

        return size;
    }

    bool File::IsOpen() const
    {
        return m_File != nullptr;
    }

    long File::CursorPos() const
    {
        return ftell(m_File);
    }
}