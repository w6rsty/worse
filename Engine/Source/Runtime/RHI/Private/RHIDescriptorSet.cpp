#include "Descriptor/RHIDescriptor.hpp"
#include "Descriptor/RHIDescriptorSet.hpp"
#include "Descriptor/RHIDescriptorSetLayout.hpp"

namespace worse
{

    RHIDescriptorSet::RHIDescriptorSet(
        RHIDescriptorSetLayout const& layout,
        std::vector<RHIDescriptor> const& descriptors, std::string_view name)
        : RHIResource(name)
    {
        update(descriptors);
    }

} // namespace worse