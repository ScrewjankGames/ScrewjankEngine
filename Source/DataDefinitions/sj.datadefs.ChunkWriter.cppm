module;
#include <ScrewjankStd/Assert.hpp>

#include <glaze/glaze.hpp>

export module sj.datadefs.ChunkWriter;
import sj.std;

export namespace sj
{
class ChunkWriter
{
public:
    ChunkWriter()
    {
    }

    template <class T>
    void Write(T&& item)
    {
        glz::error_ctx writeResult;

        if(mBuffer.empty())
            writeResult = glz::write<kWriteOpts>(std::forward<T>(item), mBuffer);
        else
            writeResult =
                glz::write_beve_append_with_delimiter<kWriteOpts>(std::forward<T>(item), mBuffer);

        SJ_ASSERT(writeResult.ec == glz::error_code::none,
                  "Failed to write chunk of type {} to buffer",
                  type_name_of<T>);
    }

    std::span<const u8> GetBufferView() const
    {
        return mBuffer;
    }

private:
    struct aligned_opts : glz::opts
    {
        bool aligned_arrays = true;
    };
    constexpr static aligned_opts kWriteOpts {glz::opts {.format = glz::BEVE}, true};

    dynamic_vector<u8> mBuffer;
};
} // namespace sj