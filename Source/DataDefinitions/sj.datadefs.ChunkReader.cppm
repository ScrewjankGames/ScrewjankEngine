module;
#include <ScrewjankStd/Assert.hpp>

#include <glaze/glaze.hpp>

#include <fstream>
#include <vector>
#include <string_view>
export module sj.datadefs.ChunkReader;
import sj.std;
export namespace sj
{
class ChunkReader
{
public:
    ChunkReader(std::string_view path, sj::memory_resource* resource)
    {
        std::ifstream sceneStream(path.data(), std::ios::in | std::ios::binary);
        mBuffer = dynamic_vector<u8>((std::istreambuf_iterator<char>(sceneStream)),
                                     std::istreambuf_iterator<char>(),
                                     resource);
    }

    template <class T>
    T Read()
    {
        T item;
        std::expected<uZ, glz::error_ctx> result = glz::read_beve_at(item, mBuffer, mCursor);
        SJ_ASSERT(result, "Failed to read chunk {} at offset {}", type_name_of<T>, mCursor);

        mCursor += *result;
        return item;
    }

    std::span<const u8> ReadUntyped()
    {
        constexpr uZ kItemDelimiter = 0x6;
        uZ begin = mCursor;
        do
        {
            mCursor++;
        } while(mCursor < mBuffer.size() && mBuffer[mCursor] != kItemDelimiter);

        return std::span(mBuffer).subspan(begin, mCursor - begin);
    }

private:
    dynamic_vector<u8> mBuffer;
    uZ mCursor = 0;
};
} // namespace sj