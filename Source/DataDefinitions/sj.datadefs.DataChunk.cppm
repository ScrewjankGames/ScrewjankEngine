module;

// Screwjank Headers
#include <ScrewjankStd/Assert.hpp>

// Library Headers
#include <glaze/glaze.hpp>

// STD Headers

export module sj.datadefs.DataChunk;
import sj.datadefs.Serialization;
import sj.std.type_info;

export namespace sj
{
    struct DataChunk
    {
        sj::TypeId type;
        glz::generic data;

        // Unpacks byte buffer into requested type
        template <class T>
        [[nodiscard]] auto Get() const -> T
        {
            T val;
            glz::error_ctx err = glz::read<glz::opts{}>(val, data);
            SJ_ASSERT(err == glz::error_code::none, "Failed to load data chunk!");

            return val;
        }
    };
} // namespace sj