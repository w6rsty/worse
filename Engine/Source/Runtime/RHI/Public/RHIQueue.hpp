#pragma once
#include "RHIDefinitions.hpp"
#include "RHIResource.hpp"
#include "RHISyncPrimitive.hpp"

#include <array>
#include <atomic>
#include <memory>

namespace worse
{

    class RHICommandList;

    class RHIQueue : public RHIResource
    {
    public:
        RHIQueue(RHIQueueType type, std::string_view name);
        ~RHIQueue();

        void wait();
        void submit(void* cmdBuffer, std::uint32_t const waitFlags,
                    RHISyncPrimitive* semaphoreWait,
                    RHISyncPrimitive* semaphoreSignal,
                    RHISyncPrimitive* semaphoreTimeline);
        void present(RHINativeHandle swapchain, std::uint32_t const imageIndex,
                     RHISyncPrimitive* semaphoreWait);
        RHICommandList* nextCommandList();
        RHIQueueType getType() const;

    private:
        std::array<std::shared_ptr<RHICommandList>, 2> m_cmdLists = {nullptr};
        std::atomic<std::uint32_t> m_index                        = 0;
        RHIQueueType m_type = RHIQueueType::Max;
        RHINativeHandle m_handle; // command pool
    };

} // namespace worse