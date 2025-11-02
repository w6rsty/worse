#pragma once
#include "base_type.hpp"
#include "RHIDefinitions.hpp"

#include <string>

namespace Worse
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
        UInt reg = 0;
        // array index
        UInt index                     = 0;
        RHIDescriptorResource resource = {};
        RHIDescriptorType type         = RHIDescriptorType::Max;
    };

    class RHIDescriptor
    {
    public:
        // clang-format off
        Bool isBindless() const { return isArray && arrayLength == 0; }
        
        // HLSL space
        UInt getSpace() const { return space; }
        // HLSL slot
        UInt getSlot() const { return slot; }
        // clang-format on

        // descriptor hash factors
        UInt space                       = 0;
        UInt slot                        = 0;
        RHIShaderStage::Flags stageFlags = RHIShaderStage::FlagBits::Unknown;

        // descriptor set has factors
        RHIDescriptorResource data = {};
        RHIDescriptorType type     = RHIDescriptorType::Max;
        RHIImageLayout layout      = RHIImageLayout::Undefined;
        //  constant buffer and buffer
        ULong range        = 0;
        UInt dynamicOffset = 0;
        // constant buffer or push constant buffer size
        UInt size        = 0;
        Bool isArray     = false;
        UInt arrayLength = 0;
        std::string name = "";
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
        RHINativeHandle allocateVariableSet(RHINativeHandle layout, UInt count);

    private:
        UInt m_expandRatio = 1;
        UInt m_rotateIndex = 0;
        std::array<std::vector<RHINativeHandle>, 2> m_pools;
        std::array<UInt, 2> m_currentPoolIndex;
    };

} // namespace Worse