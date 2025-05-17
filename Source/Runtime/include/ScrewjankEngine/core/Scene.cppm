module;

// Engine Includes
#include <ScrewjankEngine/system/memory/MemSpace.hpp>
#include <ScrewjankEngine/system/memory/allocators/PoolAllocator.hpp>

// Shared Includes
#include <ScrewjankShared/io/File.hpp>

#include <glaze/beve/read.hpp>

export module sj.core:Scene;
import sj.core.ecs;
import sj.shared.containers;
import sj.shared.datadefs;

export namespace sj
{
    class Scene
    {
    public:
        Scene(const char* path)
        {
            glz::error_ctx errorCtx = glz::read_file_beve(m_chunk, path, sj::dynamic_vector<std::byte>{});
            SJ_ASSERT(errorCtx.ec == glz::error_code::none, "Failed to load scene {}", path);


        }

        ~Scene()
        {

        }

        
    private:
        SceneChunk m_chunk;
        MemSpace<sj::FreeListAllocator> m_memSpace;
    };
}