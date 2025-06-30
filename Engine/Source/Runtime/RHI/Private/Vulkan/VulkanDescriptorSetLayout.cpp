#include "RHIDevice.hpp"
#include "RHIDescriptor.hpp"
#include "RHIDescriptorSetLayout.hpp"

namespace worse
{

    void RHIDescriptorSetLayout::nativeCreate()
    {
        // clang-format off
        std::vector<VkDescriptorSetLayoutBinding> bindings(m_descriptors.size());
        std::vector<VkDescriptorBindingFlags> bindingFlags(m_descriptors.size());

        for (std::size_t i = 0; i < m_descriptors.size(); ++i)
        {
            RHIDescriptor const& descriptor       = m_descriptors[i];
            VkDescriptorSetLayoutBinding& layoutBinding = bindings[i];

            layoutBinding.binding            = descriptor.slot;
            layoutBinding.stageFlags         = vulkanShaderStageFlags(descriptor.stageFlags);
            layoutBinding.descriptorType     = vulkanDescriptorType(descriptor.type);
            layoutBinding.descriptorCount    = descriptor.isArray ? descriptor.arrayLength : 1;
            layoutBinding.pImmutableSamplers = nullptr;

            // allow incomplete bind and update after bind
            bindingFlags[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | 
                              VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
        }

        VkDescriptorSetLayoutBindingFlagsCreateInfo infoBindingFlags = {};
        infoBindingFlags.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        infoBindingFlags.bindingCount  = static_cast<std::uint32_t>(bindingFlags.size());
        infoBindingFlags.pBindingFlags = bindingFlags.data();

        VkDescriptorSetLayoutCreateInfo infoLayout = {};
        infoLayout.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        infoLayout.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        infoLayout.pNext        = &infoBindingFlags;
        infoLayout.bindingCount = static_cast<std::uint32_t>(bindings.size());
        infoLayout.pBindings    = bindings.data();

        VkDescriptorSetLayout vkLayout = VK_NULL_HANDLE;
        WS_ASSERT_VK(vkCreateDescriptorSetLayout(RHIContext::device,
                                                 &infoLayout,
                                                 nullptr,
                                                 &vkLayout));
        m_layout = RHINativeHandle{vkLayout, RHINativeHandleType::DescriptorSetLayout};
        RHIDevice::setResourceName(m_layout, m_name);
        // clang-format on
    }

    void RHIDescriptorSetLayout::nativeDestroy()
    {
        RHIDevice::deletionQueueAdd(m_layout);
        m_layout = {};
    }

} // namespace worse