#include "RHIDescriptor.hpp"
#include "RHIDescriptorSetLayout.hpp"

namespace worse
{

    RHIDescriptorSetLayout::RHIDescriptorSetLayout(std::vector<RHIDescriptor> const& descriptors, u64 descriptorHash, std::string_view name)
        : RHIResource(name)
    {
        m_hash = descriptorHash;
        for (RHIDescriptor const& descriptor : descriptors)
        {
            if (descriptor.type == RHIDescriptorType::PushConstant)
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