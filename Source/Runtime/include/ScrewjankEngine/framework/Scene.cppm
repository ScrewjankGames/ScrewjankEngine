module;

// Shared Includes
#include <ScrewjankShared/io/File.hpp>
#include <ScrewjankShared/utils/Assert.hpp>

// Library Includes
#include <glaze/beve/read.hpp>

export module sj.engine.framework:Scene;
import sj.engine.framework.ecs;
import sj.engine.system.memory;
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