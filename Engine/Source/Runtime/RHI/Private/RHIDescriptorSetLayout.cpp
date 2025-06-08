#include "RHIDescriptorSetLayout.hpp"
#include "RHIDescriptor.hpp"
#include "Math/Hash.hpp"

namespace worse
{

    RHIDescriptorSetLayout::RHIDescriptorSetLayout(
        std::vector<RHIDescriptor> const& descriptors, std::string_view name)
        : RHIResource(name)
    {
        m_descriptors = descriptors;

        nativeCreate();

        // TODO: actually this computation is redundant, to get the this from
        // cache, we already did this in previous step
        for (RHIDescriptor const& descriptor : m_descriptors)
        {
            // clang-format off
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(descriptor.slot));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(descriptor.stageFlags));
            // clang-format on
        }
    }

    RHIDescriptorSetLayout::~RHIDescriptorSetLayout()
    {
        nativeDestroy();
    }

    void RHIDescriptorSetLayout::clearData()
    {
        UNIMPLEMENTED();
    }

} // namespace worse