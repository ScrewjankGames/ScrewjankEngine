module;

#include <memory_resource>
#include <span>

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

#include <ScrewjankStd/Assert.hpp>
export module sj.engine.rendering.vk.Primitives;
import sj.std.containers.vector;

export namespace sj
{
    inline constexpr VkAllocationCallbacks* g_vkAllocationFns = nullptr;
}

export namespace sj::vk
{
    class DescriptorLayoutBuilder
    {
    public:
        DescriptorLayoutBuilder(std::pmr::memory_resource* resource) : m_bindings(resource)
        {
        }

        void
        AddBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags shaderStageFlags)
        {
            VkDescriptorSetLayoutBinding newBind {};
            newBind.descriptorType = type;
            newBind.descriptorCount = 1;
            newBind.binding = binding;
            newBind.stageFlags = shaderStageFlags;

            m_bindings.emplace_back(newBind);
        }

        void Clear()
        {
            m_bindings.clear();
        }

        [[nodiscard]] VkDescriptorSetLayout
        Build(VkDevice device, void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0)
        {
            VkDescriptorSetLayoutCreateInfo info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
            info.pNext = pNext;

            info.pBindings = m_bindings.data();
            info.bindingCount = static_cast<uint32_t>(m_bindings.size());
            info.flags = flags;

            VkDescriptorSetLayout set;
            VkResult res = vkCreateDescriptorSetLayout(device, &info, sj::g_vkAllocationFns, &set);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to create descriptor set layout");
            return set;
        }

    private:
        dynamic_vector<VkDescriptorSetLayoutBinding> m_bindings;
    };

    class DescriptorAllocator
    {
    public:
        void InitPool(VkDevice device,
                      uint32_t maxSets,
                      std::span<VkDescriptorPoolSize> poolSizes,
                      VkDescriptorPoolCreateFlags flags)
        {
            for(VkDescriptorPoolSize& poolSize : poolSizes)
            {
                poolSize.descriptorCount *= maxSets;
            }

            VkDescriptorPoolCreateInfo pool_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
            pool_info.flags = flags;
            pool_info.maxSets = maxSets;
            pool_info.poolSizeCount = (uint32_t)poolSizes.size();
            pool_info.pPoolSizes = poolSizes.data();

            VkResult res =
                vkCreateDescriptorPool(device, &pool_info, sj::g_vkAllocationFns, &m_pool);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to create descriptor pool for global UBO");
        }

        void ResetPool(VkDevice device, VkDescriptorPoolResetFlags flags = 0)
        {
            vkResetDescriptorPool(device, m_pool, flags);
        }

        void DeInit(VkDevice device)
        {
            vkDestroyDescriptorPool(device, m_pool, sj::g_vkAllocationFns);
        }

        VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout layout)
        {
            VkDescriptorSet set; // [[indeterminate]];

            Allocate(device, std::span{&layout, 1}, std::span{&set, 1} );

            return set;
        }

        void Allocate(VkDevice device,
                      std::span<VkDescriptorSetLayout> layouts,
                      std::span<VkDescriptorSet> outBuffer)
        {
            SJ_ASSERT(outBuffer.size() >= layouts.size(),
                      "Insufficient space to record output descriptors");

            VkDescriptorSetAllocateInfo allocInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext = nullptr,
                .descriptorPool = m_pool,
                .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
                .pSetLayouts = layouts.data()
            };

            VkResult res = vkAllocateDescriptorSets(device, &allocInfo, outBuffer.data());
            SJ_ASSERT(res == VK_SUCCESS, "Failed to allocate descriptor set!");
        }

        VkDescriptorPool GetPool()
        {
            return m_pool;
        }

    private:
        VkDescriptorPool m_pool;
    };

} // namespace sj::vk