#include "RHIDescriptorSetLayout.hpp"
#include "RHIDevice.hpp"
#include "RHIDescriptor.hpp"

namespace worse
{

    void RHIDescriptorSetLayout::nativeCreate()
    {
        std::vector<RHIDescriptor> descriptors = m_descriptors;

        // exclude push constant buffers and bindless arrays
        // clang-format offP
        descriptors.erase(std::remove_if(
            descriptors.begin(),
            descriptors.end(),
            [](RHIDescriptor const& descriptor)
            {
                return descriptor.type == RHIDescriptorType::PushConstantBuffer ||
                    (descriptor.isArray && descriptor.arrayLength == 0);
            }),
            descriptors.end()
        );

        // descriptors should have unique slot, cause we use `mergeDescriptors`
        // in `getDescriptorFromPipelineState`

        // TODO: Better query this limits, for M3, the minimum is 640
        static constexpr std::size_t MAX_BINDINGS = 255;
        std::array<VkDescriptorSetLayoutBinding, MAX_BINDINGS> bindings;
        std::array<VkDescriptorBindingFlags, MAX_BINDINGS> bindingFlags;

        for (std::size_t i = 0; i < descriptors.size(); ++i)
        {
            RHIDescriptor const& descriptor       = descriptors[i];
            VkDescriptorSetLayoutBinding& binding = bindings[i];

            binding.binding            = descriptor.slot;
            binding.stageFlags         = vulkanShaderStageFlags(descriptor.stageFlags);
            binding.descriptorType     = vulkanDescriptorType(descriptor.type);
            binding.descriptorCount    = descriptor.isArray ? descriptor.arrayLength : 1;
            binding.pImmutableSamplers = nullptr;
            
            bindingFlags[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
        }
        

        VkDescriptorSetLayoutBindingFlagsCreateInfo infoBindingFlags = {};
        infoBindingFlags.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        infoBindingFlags.bindingCount  = static_cast<std::uint32_t>(descriptors.size());
        infoBindingFlags.pBindingFlags = bindingFlags.data();
        
        VkDescriptorSetLayoutCreateInfo infoLayout = {};
        infoLayout.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        infoLayout.pNext        = &infoBindingFlags;
        infoLayout.bindingCount = static_cast<std::uint32_t>(descriptors.size());
        infoLayout.pBindings    = bindings.data();
        
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        WS_ASSERT_VK(vkCreateDescriptorSetLayout(RHIContext::device,
                                                 &infoLayout,
                                                 nullptr,
                                                 &layout));
        m_handle = RHINativeHandle{layout, RHINativeHandleType::DescriptorSetLayout};
        RHIDevice::setResourceName(m_handle, m_name);
        // clang-format on
    }

    void RHIDescriptorSetLayout::nativeDestroy()
    {
        RHIDevice::deletionQueueAdd(m_handle);
        m_handle = {};
    }

} // namespace worse