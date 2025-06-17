#include "Math/Hash.hpp"
#include "Descriptor/RHIBuffer.hpp"
#include "Descriptor/RHIDescriptor.hpp"
#include "Descriptor/RHIDescriptorSet.hpp"
#include "Descriptor/RHIDescriptorSetLayout.hpp"

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

    void RHIDescriptorSetLayout::setConstantBuffer(RHIBuffer* buffer,
                                                   std::uint32_t const slot)
    {
        for (RHIDescriptor& descriptor : m_descriptors)
        {
            if (descriptor.slot == (slot))
            {
                descriptor.data.buffer   = buffer;
                descriptor.range         = buffer->getStride();
                descriptor.dynamicOffset = buffer->getOffset();
                return;
            }
        }
    }

    void RHIDescriptorSetLayout::setBuffer(RHIBuffer* buffer,
                                           std::uint32_t const slot)
    {
        for (RHIDescriptor& descriptor : m_descriptors)
        {
            if (descriptor.slot == (slot))
            {
                descriptor.data.buffer   = buffer;
                descriptor.range         = buffer->getStride();
                descriptor.dynamicOffset = buffer->getOffset();
                return;
            }
        }
    }

    void RHIDescriptorSetLayout::clearData()
    {
        UNIMPLEMENTED();
    }

    RHIDescriptorSet* RHIDescriptorSetLayout::getDescriptorSet() const
    {
        
    }

} // namespace worse