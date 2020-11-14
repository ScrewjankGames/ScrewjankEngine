// STD Headers

// Library Headers

// Screwjank Headers
#include "rendering/Renderer.hpp"
#include "rendering/RendererAPI.hpp"
#include "core/Log.hpp"
#include "core/Assert.hpp"

namespace sj {

    Renderer::Renderer()
    {
        m_RendererAPI = RendererAPI::Create();
    }

    Renderer::~Renderer()
    {
    }

} // namespace sj
