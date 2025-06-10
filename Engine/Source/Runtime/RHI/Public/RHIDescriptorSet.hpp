#pragma once
#include "RHIResource.hpp"

#include <string_view>
#include <vector>

namespace worse
{

    class RHIDescriptorSet : public RHIResource
    {
        void update(std::vector<RHIDescriptor> const& descriptors);

    public:
        RHIDescriptorSet() = default;
        RHIDescriptorSet(RHIDescriptorSetLayout const& layout,
                         std::vector<RHIDescriptor> const& descriptors,
                         std::string_view name);
        ~RHIDescriptorSet() = default;

        // clang-format off
        RHINativeHandle getHandle() const { return m_handle; }
        // clang-format on

    private:
        std::vector<RHIDescriptor> m_descriptors;
        RHINativeHandle m_handle;
    };

} // namespace worse