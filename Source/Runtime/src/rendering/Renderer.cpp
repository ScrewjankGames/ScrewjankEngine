// STD Headers

// Library Headers

// Screwjank Headers
#include <ScrewjankEngine/utils/Assert.hpp>
#include <ScrewjankEngine/utils/Log.hpp>
#include <ScrewjankEngine/core/Window.hpp>
#include <ScrewjankEngine/rendering/Renderer.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanRendererAPI.hpp>

namespace sj
{
    MemSpace<FreeListAllocator>* Renderer::WorkBuffer()
    {
        static MemSpace zone(MemorySystem::GetRootMemSpace(), 8 * k1_KiB, "Renderer Work Buffer");
        return &zone;
    }

    Renderer::Renderer() : m_renderAPI(VulkanRendererAPI::GetInstance())
    {
        m_renderAPI->Init();
    }

    void Renderer::Render()
    {
        m_renderAPI->Draw();
    }

    Renderer::~Renderer()
    {
    }

} // namespace sj
