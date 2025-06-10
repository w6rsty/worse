#include "RHIDevice.hpp"
#include "RHIDescriptor.hpp"
#include "RHIDescriptorSet.hpp"
#include "RHIDescriptorSetLayout.hpp"

namespace worse
{

    RHIDescriptorSet::RHIDescriptorSet(
        RHIDescriptorSetLayout const& layout,
        std::vector<RHIDescriptor> const& descriptors, std::string_view name)
        : RHIResource(name)
    {
        m_handle = RHIDevice::allocateDescriptorSet(layout);
        RHIDevice::setResourceName(m_handle, name);

        update(descriptors);
    }

} // namespace worse