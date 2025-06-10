#pragma once
#include "RHIDefinitions.hpp"

#include <cstdint>
#include <string>

namespace worse
{
    struct RHIDescriptorPayload
    {
        union
        {
            void* raw = nullptr;
            RHIBuffer* buffer;
            RHITexture* texture;
        };
    };

    class RHIDescriptor
    {
    public:
        // clang-format off
        bool isBindless() const { return isArray && arrayLength == 0; }

        // descriptor hash factors
        std::uint32_t slot             = 0;
        RHIShaderStageFlags stageFlags = RHIShaderStageFlagBits::None;

        // descriptor set has factors
        RHIDescriptorPayload data   = {};
        RHIDescriptorType type      = RHIDescriptorType::Max;
        RHIImageLayout layout       = RHIImageLayout::Undefined;
        //  constant buffer and buffer
        std::uint64_t range         = 0;
        std::uint32_t dynamicOffset = 0;
        // constant buffer or push constant buffer size
        std::uint32_t size          = 0;
        bool isArray                = false;
        std::uint32_t arrayLength   = 0;
        std::string name            = "";
        // clang-format on
    };

} // namespace worse