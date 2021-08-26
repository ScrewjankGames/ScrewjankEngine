// STD Headers

// Library Headers

// Screwjank Headers
#include <core/Assert.hpp>
#include <core/Log.hpp>
#include <core/Window.hpp>
#include <rendering/Renderer.hpp>
#include <rendering/RendererAPI.hpp>

namespace sj {

    Renderer::Renderer(Window* window)
    {
        m_RendererAPI = RendererAPI::Create(window);
    }

    Renderer::~Renderer()
    {
    }

} // namespace sj
