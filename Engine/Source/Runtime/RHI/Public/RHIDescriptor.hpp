#pragma once
#include "RHIDefinitions.hpp"

#include <cstdint>
#include <string>

namespace worse
{
    struct RHIDescriptor
    {
        std::string name = "";

        std::uint32_t slot              = 0;
        RHIShaderStageFlags stageFlags = RHIShaderStageFlagBits::None;

        RHIDescriptorType type = RHIDescriptorType::Max;
        RHIImageLayout layout  = RHIImageLayout::Undefined;
        // constant buffer or push constant buffer size
        std::uint32_t size        = 0;
        bool isArray              = false;
        std::uint32_t arrayLength = 0;
    };

} // namespace worse