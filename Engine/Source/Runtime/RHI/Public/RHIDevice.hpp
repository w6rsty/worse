#pragma once
#include "Types.hpp"
#include "RHIResource.hpp"
#include "Descriptor/RHIDescriptor.hpp"

#include <span>

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
        // clang-format on

        // reset allocated descriptors
        static void resetDescriptorAllocator();
        // expose for pipeline layout creation
        static RHINativeHandle getGlobalDescriptorSetLayout();
        static RHINativeHandle getGlobalDescriptorSet();
        // allocate and write global descriptor set
        static void writeGlobalDescriptorSet();
        static void
        updateBindless(RHIBindlessResourceType const type,
                       std::span<RHIDescriptorBindlessWrite> updates);
        static RHINativeHandle
        getBindlessDescriptorSetLayout(RHIBindlessResourceType const type);
        static RHINativeHandle
        getBindlessSet(RHIBindlessResourceType const type,
                       std::span<RHIDescriptorBindlessWrite> updates);

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