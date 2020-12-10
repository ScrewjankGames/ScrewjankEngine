// STD Headers

// Library Headers

// Screwjank Headers
#include "rendering/RenderDevice.hpp"
#include "rendering/RendererAPI.hpp"
#include "platform/Vulkan/VulkanRenderDevice.hpp"

namespace sj {

    UniquePtr<RenderDevice> sj::RenderDevice::Create()
    {
        auto vendor_api = RendererAPI::GetVendorAPI();

        SJ_ASSERT(vendor_api != RendererAPI::API::Unkown,
                  "Unknown rendering API. Cannot create device.");

        RenderDevice* render_device = nullptr;

        if (vendor_api == RendererAPI::API::Vulkan) {
            render_device = New<VulkanRenderDevice>();
        } else {
            ;
        }

        SJ_ASSERT(render_device != nullptr, "Failed to create render device");

        return UniquePtr<RenderDevice>(render_device, [](RenderDevice* ptr) {
            Delete(ptr);
        });
    }

} // namespace sj
