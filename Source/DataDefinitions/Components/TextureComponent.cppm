module;

// Shared Includes
#include <ScrewjankStd/TypeMacros.hpp>

export module sj.datadefs:TextureComponent;
import sj.std.math;

export namespace sj
{
    struct TextureComponentChunk
    {
        
    };

    struct TextureComponent
    {
        SJ_STRUCT_TYPE_ID(TextureComponent);
        bool foo;
    };
} // namespace sj