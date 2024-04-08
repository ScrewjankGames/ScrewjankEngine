// STD Headers

// Library Headers

// Screwjank Headers
#include <ScrewjankEngine/utils/Assert.hpp>
#include <ScrewjankEngine/utils/Log.hpp>
#include <ScrewjankEngine/core/Window.hpp>
#include <ScrewjankEngine/rendering/Renderer.hpp>
#include <ScrewjankEngine/rendering/RendererAPI.hpp>

namespace sj
{
    MemSpace<FreeListAllocator>* Renderer::WorkBuffer()
    {
        static MemSpace zone(MemorySystem::GetRootMemSpace(), 8 * k1_KiB, "Renderer Work Buffer");
        return &zone;
    }

    Renderer::Renderer()
    {
        m_RendererAPI = RendererAPI::Create();
    }

    void Renderer::Render()
    {
        m_RendererAPI->DrawFrame();
    }

    Renderer::~Renderer()
    {
    }

} // namespace sj
