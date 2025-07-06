module;

// Screwjank Headers
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/TypeMacros.hpp>

// Library Headers
#include <glaze/glaze.hpp>

// STD Headers

export module sj.datadefs.DataChunk;
import sj.datadefs.ChunkTypes;
import sj.datadefs.Serialization;
import sj.std.containers;

export namespace sj
{
    struct DataChunk
    {
        sj::TypeId type;
        sj::dynamic_vector<std::byte> data;

        // Unpacks byte buffer into requested type
        template <class T>
        [[nodiscard]] auto Get() const -> T
        {
            T val;

            glz::error_ctx err = glz::read_beve(val, data);
            SJ_ASSERT(err == glz::error_code::none, "Failed to load data chunk!");

            return val;
        }
    };

    template<class T>
    void WriteChunk(const glz::json_t& data, DataChunk& out_chunk)
    {
        out_chunk.type = T::kTypeId;

        T val;
        glz::error_ctx err = glz::read_json<T>(val, data);
        SJ_ASSERT(err.ec == glz::error_code::none, "Failed to parse chunk JSON!");

        err = glz::write_beve<T>(std::move(val), out_chunk.data);
        SJ_ASSERT(err.ec == glz::error_code::none, "Failed to write chunk BEVE!");
    }

    template<class T>
    void ReadChunk(const DataChunk& chunkData, void* out_chunkPtr)
    {
        SJ_ASSERT(chunkData.type == T::kTypeId, "Chunk type mismatch!");

        T& chunk = *reinterpret_cast<T*>(out_chunkPtr);

        glz::error_ctx err = glz::read_beve(chunk, chunkData.data);
        SJ_ASSERT(err == glz::error_code::none, "Failed to read data chunk");
    }

    using WriteChunkFn = void (*)(const glz::json_t&, DataChunk&);
    constexpr type_map<
        g_componentChunkTypes,
        TypeId,
        WriteChunkFn,
        []<class T>() -> TypeId { return T::kTypeId; },
        []<class T>() -> WriteChunkFn { return WriteChunk<T>; }>
    g_chunkWriterFuncs;


    using ReadChunkFn = void (*)(const DataChunk&, void*);
    constexpr type_map<
        g_componentChunkTypes,
        TypeId,
        ReadChunkFn,
        []<class T>() -> TypeId { return T::kTypeId; },
        []<class T>() -> ReadChunkFn { return ReadChunk<T>; }>
    g_chunkReaderFuncs;
} // namespace sj