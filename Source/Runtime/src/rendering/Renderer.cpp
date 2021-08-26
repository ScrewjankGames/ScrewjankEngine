// STD Headers

// Library Headers

// Screwjank Headers
#include <ScrewjankEngine/core/Assert.hpp>
#include <ScrewjankEngine/core/Log.hpp>
#include <ScrewjankEngine/core/Window.hpp>
#include <ScrewjankEngine/rendering/Renderer.hpp>
#include <ScrewjankEngine/rendering/RendererAPI.hpp>

namespace sj {

    Renderer::Renderer(Window* window)
    {
        m_RendererAPI = RendererAPI::Create(window);
    }

    Renderer::~Renderer()
    {
    }

} // namespace sj
