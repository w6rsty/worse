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
        u32 reg = 0;
        // array index
        u32 index                      = 0;
        RHIDescriptorResource resource = {};
        RHIDescriptorType type         = RHIDescriptorType::Max;
    };

    class RHIDescriptor
    {
    public:
        // clang-format off
        bool isBindless() const { return isArray && arrayLength == 0; }
        
        // HLSL space
        u32 getSpace() const { return space; }
        // HLSL slot
        u32 getSlot() const { return slot; }

        // descriptor hash factors
        u32 space            = 0;
        u32 slot             = 0;
        RHIShaderStageFlags stageFlags = RHIShaderStageFlagBits::None;

        // descriptor set has factors
        RHIDescriptorResource data  = {};
        RHIDescriptorType type      = RHIDescriptorType::Max;
        RHIImageLayout layout       = RHIImageLayout::Undefined;
        //  constant buffer and buffer
        u64 range         = 0;
        u32 dynamicOffset = 0;
        // constant buffer or push constant buffer size
        u32 size          = 0;
        bool isArray                = false;
        u32 arrayLength   = 0;
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
        RHINativeHandle allocateVariableSet(RHINativeHandle layout, u32 count);

    private:
        u32 m_expandRatio = 1;
        u32 m_rotateIndex = 0;
        std::array<std::vector<RHINativeHandle>, 2> m_pools;
        std::array<u32, 2> m_currentPoolIndex;
    };

} // namespace worse