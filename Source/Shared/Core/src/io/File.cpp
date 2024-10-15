// Parent
#include <ScrewjankShared/IO/File.hpp>

// Screwjank Headers
#include <ScrewjankShared/utils/Assert.hpp>

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

        fopen_s(&m_File, path, modeString);
       
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
        if (!m_File)
        {
            return 0;
        }

        // Save off cursor position
        uint64_t cursorPos = ftell(m_File);

        // Seek to end of file
        fseek(m_File, 0, SEEK_END);
        uint64_t size = ftell(m_File);

        // Restore cursor position
        fseek(m_File, 0, (int)cursorPos);

        return size;
    }

    bool File::IsOpen() const
    {
        return m_File != nullptr;
    }

    size_t File::CursorPos() const
    {
        return ftell(m_File);
    }
}