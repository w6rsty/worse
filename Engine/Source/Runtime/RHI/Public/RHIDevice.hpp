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

        // wait graphics and compute queues
        static void queueWaitAll();
        static std::uint32_t getQueueIndex(RHIQueueType const type);
        static RHIQueue* getQueue(RHIQueueType const type);
        static RHINativeHandle getQueueRHIResource(RHIQueueType const type);

        // reset allocated descriptors
        static void resetDescriptorAllocator();
        // expose for pipeline layout creation
        static RHINativeHandle getGlobalDescriptorSetLayout();
        static RHINativeHandle getGlobalDescriptorSet();
        // allocate and write global descriptor set
        static void writeGlobalDescriptorSet();
        static void
        updateBindlessTextures(std::span<RHIDescriptorWrite> updates);
        // used by RHIPipelinePool to create pipeline layout
        static RHIDescriptorSetLayout*
        getSpecificDescriptorSetLayout(RHIPipelineState const& pso);
        static RHINativeHandle
        getSpecificDescriptorSet(std::uint64_t descriptorHash);
        static void resetSpecificDescriptorSets();

        // get descriptor set layout from pool or create a new one
        static RHIPipeline* getPipeline(RHIPipelineState const& pso);

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

        static RHICommandList* cmdImmediateBegin(RHIQueueType const type);
        static void cmdImmediateSubmit(RHICommandList* cmdList);

        static void setResourceName(RHINativeHandle const& resource,
                                    std::string_view name);
    };
}; // namespace worse