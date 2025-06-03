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

        static void deletionQueueAdd(RHINativeHandle const& resource);
        static void deletionQueueFlush();

        static void setResourceName(RHINativeHandle const& resource,
                                    std::string_view name);
    };
}; // namespace worse