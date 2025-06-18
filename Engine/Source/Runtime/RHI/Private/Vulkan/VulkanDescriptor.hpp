#pragma once
#include "RHIResource.hpp"
#include "Descriptor/RHIDescriptor.hpp"

#include <span>
#include <array>
#include <vector>

namespace worse
{

    class VulkanDescriptorAllocator
    {
        VkDescriptorPool createPool();
        RHINativeHandle allocateInternal(VkDescriptorSetLayout layout,
                                         void* pNext);

    public:
        VulkanDescriptorAllocator();
        ~VulkanDescriptorAllocator();

        void resetAll();

        RHINativeHandle allocateSet(RHINativeHandle layout);
        RHINativeHandle allocateVariableSet(RHINativeHandle layout,
                                            std::uint32_t count);

    private:
        std::uint32_t m_expandRatio = 1;
        std::uint32_t m_rotateIndex = 0;
        std::array<std::vector<VkDescriptorPool>, 2> m_pools;
        std::array<std::uint32_t, 2> m_currentPoolIndex;
    };

    class VulkanGlobalSet
    {
        void createLayout();
        void createInfos();

    public:
        VulkanGlobalSet(VulkanDescriptorAllocator* allocator);

        ~VulkanGlobalSet();

        // allocate set and write all descriptor, for bindless textures are
        // wrote with default placeholder texture, call at frame start
        void writeAll();
        // write bindless textures
        // write and update bindless textures at runtime
        void
        writeBindlessTextures(std::span<RHIBindlessDescriptorWrite> updates);

        // clang-format off
        RHINativeHandle getLayout() const { return m_layout; }
        RHINativeHandle getSet() const    { return m_set; }
        // clang-format on

    private:
        bool m_firstUpdate                     = true;
        VulkanDescriptorAllocator* m_allocator = nullptr;
        RHINativeHandle m_layout               = {};
        RHINativeHandle m_set                  = {};

        // clang-format off
        VkDescriptorBufferInfo m_frameConstantBufferInfo = {};
        EnumArray<RHISamplerType, VkDescriptorImageInfo> m_samplerInfos = {};
        std::array<VkWriteDescriptorSet, 3> m_staticWrites = {};
        // clang-format on
    };

} // namespace worse