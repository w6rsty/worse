#include "Descriptor/RHIDescriptor.hpp"
#include "Descriptor/RHIDescriptorSetLayout.hpp"

namespace worse
{

    RHIDescriptorSetLayout::RHIDescriptorSetLayout(
        std::vector<RHIDescriptor> const& descriptors,
        std::uint64_t descriptorHash, std::string_view name)
        : RHIResource(name)
    {
        m_hash = descriptorHash;
        for (RHIDescriptor const& descriptor : descriptors)
        {
            if (descriptor.type == RHIDescriptorType::PushConstantBuffer)
            {
                m_pushConstants.push_back(descriptor);
            }
            else
            {
                m_descriptors.push_back(descriptor);
            }
        }

        nativeCreate();
    }

    RHIDescriptorSetLayout::~RHIDescriptorSetLayout()
    {
        nativeDestroy();
    }

} // namespace worse