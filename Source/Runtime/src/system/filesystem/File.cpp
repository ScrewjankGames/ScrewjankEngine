// Parent
#include <ScrewjankEngine/system/filesystem/File.hpp>

// Screwjank Headers
#include <ScrewjankEngine/core/Assert.hpp>

sj::File::~File()
{
    SJ_ASSERT(m_File == nullptr, "File not closed!");
}

bool sj::File::Open(const char* path, OpenMode mode)
{
    SJ_ASSERT(m_File == nullptr, "Close file before opening something else!");

    const char* modeString = [&mode]() 
    {
        switch(mode)
        {
        case sj::File::OpenMode::kRead:
            return "r";
        case sj::File::OpenMode::kWrite:
            return "w";
        case sj::File::OpenMode::kReadBinary:
            return "rb";
        case sj::File::OpenMode::kWriteBinary:
            return "wb";
        default:
            return "";
            break;
        }
    }();

    fopen_s(&m_File, path, modeString);
       
    return m_File != nullptr;
}

void sj::File::Close()
{
    if(m_File)
    {
        fclose(m_File);
        m_File = nullptr;
    }
}

size_t sj::File::Read(void* buffer, size_t size, size_t count)
{
    size_t blockCount = fread(buffer, size, count, m_File);
    return blockCount * size;
}

size_t sj::File::Write(const void* buffer, size_t size, size_t count)
{
    size_t blockCount = fwrite(buffer, size, count, m_File);
    return blockCount * size;
}

uint64_t sj::File::Size()
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

bool sj::File::IsOpen() const
{
    return m_File != nullptr;
}
