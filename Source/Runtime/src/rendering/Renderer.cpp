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

    void Renderer::Render()
    {
        m_renderAPI->Draw();
    }

    void Renderer::Init() 
    {
        SJ_ASSERT(!m_renderAPI, "Double renderer init detected");
        m_renderAPI = VulkanRendererAPI::GetInstance();
        m_renderAPI->Init();
    }

    void Renderer::DeInit()
    {
        m_renderAPI->DeInit();
    }

    void Renderer::StartRenderFrame()
    {
        m_renderAPI->StartRenderFrame();
    }

} // namespace sj
