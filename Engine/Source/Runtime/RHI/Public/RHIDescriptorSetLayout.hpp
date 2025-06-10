#pragma once
#include "RHIResource.hpp"

#include <vector>
#include <cstdint>
#include <string_view>

namespace worse
{

    class RHIDescriptorSetLayout : public RHIResource
    {
        void nativeCreate();
        void nativeDestroy();

    public:
        RHIDescriptorSetLayout() = default;
        RHIDescriptorSetLayout(std::vector<RHIDescriptor> const& descriptors,
                               std::string_view name);
        ~RHIDescriptorSetLayout();

        void setConstantBuffer(RHIBuffer* buffer, std::uint32_t slot);
        void clearData();
        RHIDescriptorSet* getDescriptorSet() const;

        // clang-format off
        std::vector<RHIDescriptor> const& getDescriptors() const { return m_descriptors; }
        std::uint64_t getHash() const                            { return m_hash; }
        RHINativeHandle getHandle() const                        { return m_handle; }
        // clang-format on

    private:
        std::vector<RHIDescriptor> m_descriptors;
        std::uint64_t m_hash = 0;
        RHINativeHandle m_handle;
    };

} // namespace worse