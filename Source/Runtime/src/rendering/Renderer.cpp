// STD Headers

// Library Headers

// Screwjank Headers
#include <ScrewjankEngine/core/Assert.hpp>
#include <ScrewjankEngine/core/Log.hpp>
#include <ScrewjankEngine/core/Window.hpp>
#include <ScrewjankEngine/rendering/Renderer.hpp>
#include <ScrewjankEngine/rendering/RendererAPI.hpp>

namespace sj
{
    HeapZone* Renderer::WorkBuffer()
    {
        static HeapZone zone(MemorySystem::GetRootHeapZone(), k1_KiB, "Renderer Work Buffer");
        return &zone;
    }

    Renderer::Renderer()
    {
        m_RendererAPI = RendererAPI::Create();
    }

    void Renderer::Render()
    {

    }

    Renderer::~Renderer()
    {
    }

} // namespace sj
