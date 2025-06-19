#pragma once
#include "RHIResource.hpp"
#include "Descriptor/RHIDescriptor.hpp"

#include <span>
#include <vector>
#include <string_view>

namespace worse
{

    // pipeline specific set 1 layout and push constants
    class RHIDescriptorSetLayout : public RHIResource
    {
        void nativeCreate();
        void nativeDestroy();

    public:
        RHIDescriptorSetLayout() = default;
        RHIDescriptorSetLayout(std::vector<RHIDescriptor> const& descriptors,
                               std::uint64_t descriptorHash,
                               std::string_view name);
        ~RHIDescriptorSetLayout();

        void updateSet(std::span<RHIDescriptorWrite> constantBuffers,
                       std::span<RHIDescriptorWrite> rwBuffers,
                       std::span<RHIDescriptorWrite> textures);

        // clang-format off
        std::vector<RHIDescriptor> const& getDescriptors() const   { return m_descriptors; }
        std::vector<RHIDescriptor> const& getPushConstants() const { return m_pushConstants; }
        std::uint64_t                     getHash() const          { return m_hash; }
        RHINativeHandle                   getLayout() const        { return m_layout; }
        // clang-format on

    private:
        std::vector<RHIDescriptor> m_descriptors;
        std::vector<RHIDescriptor> m_pushConstants;

        std::uint64_t m_hash     = 0;
        RHINativeHandle m_layout = {};
    };

} // namespace worse