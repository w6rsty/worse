#pragma once
#include "Types.hpp"
#include "RHIDefinitions.hpp"
#include "RHIResource.hpp"

namespace worse
{
    class RHIQueue;

    class RHIDevice : public NonCopyable, public NonMovable
    {
    public:
        static void initialize();
        static void destroy();

        // wait graphics and compute queues
        static void queueWaitAll();
        static std::uint32_t getQueueIndex(RHIQueueType const type);
        static RHIQueue* getQueue(RHIQueueType const type);
        static RHINativeHandle getQueueRHIResource(RHIQueueType const type);

        static RHINativeHandle
        getBindlessDescriptorSetLayout(RHIBindlessResourceType const type);

        // use pso hash to get pipeline and descriptor set layout from cache
        // guaranteed to return a valid pipeline and descriptor set layout
        static void
        getOrCreatePipeline(RHIPipelineState const& pso, RHIPipeline*& pipeline,
                            RHIDescriptorSetLayout*& descriptorSetLayout);

        static void memoryTextureCreate(RHITexture* texture);
        static void memoryTextureDestroy(RHINativeHandle handle);

        static void deletionQueueAdd(RHINativeHandle const& resource);
        static void deletionQueueFlush();

        static RHICommandList* CmdImmediateBegin(RHIQueueType const type);
        static void CmdImmediateSubmit(RHICommandList* cmdList);

        static void setResourceName(RHINativeHandle const& resource,
                                    std::string_view name);
    };
}; // namespace worse