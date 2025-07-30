#pragma once
#include "RHIResource.hpp"
#include "RHIDescriptor.hpp"

#include <span>
#include <array>
#include <unordered_map>

namespace worse
{

    class VulkanGlobalSet
    {
        void createLayout();
        // create allocation and write infos
        void createInfos();

    public:
        VulkanGlobalSet(RHIDescriptorAllocator* allocator);

        ~VulkanGlobalSet();

        // allocate set and write all descriptor, for bindless textures are
        // wrote with default placeholder texture, call at frame start
        void writeStatic();
        // write bindless textures
        // write and update bindless textures at runtime
        void writeBindlessTextures(std::span<RHIDescriptorWrite> updates);

        // clang-format off
        RHINativeHandle getLayout() const { return m_layout; }
        RHINativeHandle getSet() const    { return m_set; }
        // clang-format on

    private:
        bool m_firstUpdate                  = true;
        RHIDescriptorAllocator* m_allocator = nullptr;
        RHINativeHandle m_layout            = {};
        RHINativeHandle m_set               = {};

        // clang-format off
        VkDescriptorBufferInfo m_frameConstantBufferInfo = {};
        EnumArray<RHISamplerType, VkDescriptorImageInfo> m_samplerInfos = {};
        std::array<VkWriteDescriptorSet, 3> m_staticWrites = {};
        // clang-format on
    };

    class VulkanSpecificSet
    {
    public:
        VulkanSpecificSet(RHIDescriptorAllocator* allocator);
        ~VulkanSpecificSet();

        // get descriptor set layout for pipeline state, or create a new one
        RHIDescriptorSetLayout*
        getDescriptorSetLayout(RHIPipelineState const& pso);
        // get descriptor set layout for specific hash, return nullptr if not
        // found
        RHIDescriptorSetLayout* getDescriptorSetLayout(u64 hash);
        // get descriptor set, or allocate a new one
        RHINativeHandle getDescriptorSet(u64 hash);

        // sets will be reset every frame, must invalidate cache
        void resetSets();

    private:
        RHIDescriptorAllocator* m_allocator = nullptr;
        // clang-format off
        std::unordered_map<u64, std::shared_ptr<RHIDescriptorSetLayout>> m_descriptorSetLayouts;
        std::unordered_map<u64, RHINativeHandle> m_descriptorSets;
        // clang-format on
    };

} // namespace worse