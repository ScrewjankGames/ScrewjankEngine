module;

// Screwjank Headers
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/TypeMacros.hpp>

// Library Headers
#include <glaze/glaze.hpp>

export module sj.shared.datadefs.DataChunk;
import sj.std.containers;
import sj.std.math;

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
            SJ_ASSERT(err == glz::error_code::none, "Failed to load data chunk");

            return val;
        }
    };
} // namespace sj