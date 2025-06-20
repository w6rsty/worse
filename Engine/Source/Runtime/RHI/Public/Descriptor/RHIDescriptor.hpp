#pragma once
#include "RHIDefinitions.hpp"

#include <cstdint>
#include <string>

namespace worse
{

    namespace GlobalDescriptorSetLayout
    {
        enum class GlobalBinding
        {
            FrameConstant,
            SamplerComparison,
            SamplerRegulars,
        };

        void createGlobalDescriptorSetLayout();
        RHINativeHandle getGlobalDescriptorSetLayout();
    } // namespace GlobalDescriptorSetLayout

    namespace BindlessDescriptorLayout
    {
        enum class BindlessBinding
        {
            MaterialTextures,
            MaterialProperties,
            LightProperties,
        };

        void createBindlessDescriptorSetLayout();
        RHINativeHandle getBindlessDescriptorSetLayout();
    } // namespace BindlessDescriptorLayout

    struct RHIDescriptorResource
    {
        union
        {
            void* raw = nullptr;
            RHIBuffer* buffer;
            RHITexture* texture;
        };
    };

    struct RHIDescriptorWrite
    {
        // for bindless keep this 0
        std::uint32_t reg = 0;
        // array index
        std::uint32_t index            = 0;
        RHIDescriptorResource resource = {};
        RHIDescriptorType type         = RHIDescriptorType::Max;
    };

    class RHIDescriptor
    {
    public:
        // clang-format off
        bool isBindless() const { return isArray && arrayLength == 0; }
        
        // HLSL space
        std::uint32_t getSpace() const { return space; }
        // HLSL slot
        std::uint32_t getSlot() const { return slot; }

        // descriptor hash factors
        std::uint32_t space            = 0;
        std::uint32_t slot             = 0;
        RHIShaderStageFlags stageFlags = RHIShaderStageFlagBits::None;

        // descriptor set has factors
        RHIDescriptorResource data  = {};
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

    class RHIDescriptorAllocator
    {
        RHINativeHandle createPool();
        RHINativeHandle allocateInternal(RHINativeHandle layout, void* pNext);

    public:
        RHIDescriptorAllocator();
        ~RHIDescriptorAllocator();

        void resetAll();

        RHINativeHandle allocateSet(RHINativeHandle layout);
        RHINativeHandle allocateVariableSet(RHINativeHandle layout,
                                            std::uint32_t count);

    private:
        std::uint32_t m_expandRatio = 1;
        std::uint32_t m_rotateIndex = 0;
        std::array<std::vector<RHINativeHandle>, 2> m_pools;
        std::array<std::uint32_t, 2> m_currentPoolIndex;
    };

} // namespace worse