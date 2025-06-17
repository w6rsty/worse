#pragma once
#include "RHIResource.hpp"
#include "RHISyncPrimitive.hpp"

#include <array>
#include <atomic>
#include <memory>

namespace worse
{

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

        // clang-format off
        std::uint32_t getIndex() const { return m_index; }
        RHIQueueType  getType() const  { return m_type; }
        // clang-format on

    private:
        std::array<std::shared_ptr<RHICommandList>, 2> m_cmdLists = {nullptr};
        std::atomic<std::uint32_t> m_index                        = 0;
        RHIQueueType m_type = RHIQueueType::Max;
        RHINativeHandle m_handle; // command pool
    };

} // namespace worse