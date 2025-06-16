#pragma once
#include "Types.hpp"
#include "RHIResource.hpp"

namespace worse
{

    class RHIDevice : public NonCopyable, public NonMovable
    {
    public:
        static void initialize();
        static void destroy();

        // Renderer set this after RHIDevice is initialized
        static void setResourceProvider(RHIResourceProvider* provider);
        static RHIResourceProvider* getResourceProvider();

        // clang-format off

        // wait graphics and compute queues
        static void queueWaitAll();
        [[nodiscard]] static std::uint32_t getQueueIndex(RHIQueueType const type);
        [[nodiscard]] static RHIQueue* getQueue(RHIQueueType const type);
        [[nodiscard]] static RHINativeHandle getQueueRHIResource(RHIQueueType const type);
        [[nodiscard]] static RHINativeHandle allocateDescriptorSet(RHIDescriptorSetLayout const& layout);
        // dynamic descriptor sets
        [[nodiscard]] static RHINativeHandle getGlobalDescriptorSetLayout();
        [[nodiscard]] static RHINativeHandle getGlobalDescriptorSet();
        [[nodiscard]] static std::unordered_map<std::uint64_t, RHIDescriptorSet>& getDescriptorSets();
        [[nodiscard]] static RHINativeHandle getBindlessDescriptorSet(RHIBindlessResourceType const type);
        [[nodiscard]] static RHINativeHandle getBindlessDescriptorSetLayout(RHIBindlessResourceType const type);
        // update descriptors
        static void updateGlobalDescriptorSet();
        static void updateBindlessResources();

        // clang-format on

        // use pso hash to get pipeline and descriptor set layout from cache
        // guaranteed to return a valid pipeline and descriptor set layout
        static void
        getOrCreatePipeline(RHIPipelineState const& pso, RHIPipeline*& pipeline,
                            RHIDescriptorSetLayout*& descriptorSetLayout);

        static void memoryTextureCreate(RHITexture* texture);
        static void memoryTextureDestroy(RHINativeHandle handle);
        static RHINativeHandle memoryBufferCreate(std::uint32_t size,
                                                  std::uint32_t bufferUsage,
                                                  std::uint32_t memoryProperty,
                                                  void const* data,
                                                  std::string_view name);
        static void memoryBufferDestroy(RHINativeHandle handle);
        static void* memoryGetMappedBufferData(RHINativeHandle handle);

        static void deletionQueueAdd(RHINativeHandle const& resource);
        static void deletionQueueFlush();

        [[nodiscard]] static RHICommandList*
        cmdImmediateBegin(RHIQueueType const type);
        static void cmdImmediateSubmit(RHICommandList* cmdList);

        static void setResourceName(RHINativeHandle const& resource,
                                    std::string_view name);
    };
}; // namespace worse