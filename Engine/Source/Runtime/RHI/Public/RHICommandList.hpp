#pragma once
#include "RHIDefinitions.hpp"
#include "RHIResource.hpp"

#include <cstdint>
#include <atomic>

namespace worse
{

    class RHIQueue;
    class RHISyncPrimitive;

    enum class RHICommandListState
    {
        Idle,
        Recording,
        Submitted,
    };

    struct ImageBarrierInfo
    {
        void* image               = nullptr;
        std::uint32_t aspectMask  = 0;
        std::uint32_t mipIndex    = 0;
        std::uint32_t mipRange    = 0;
        std::uint32_t arrayLength = 0;
        RHIImageLayout layoutOld  = RHIImageLayout::Max;
        RHIImageLayout layoutNew  = RHIImageLayout::Max;
        bool isDepth              = false;
    };

    class RHICommandList
    {
    public:
        RHICommandList(RHIQueue* queue, RHIResource cmdPool, char const* name);
        ~RHICommandList();

        void begin();
        void submit(RHISyncPrimitive* semaphoreWait);
        void waitForExecution();

        void insertBarrier(RHIResource image, RHIFormat const format,
                           std::uint32_t const mipIndex,
                           std::uint32_t const mipRange,
                           std::uint32_t const arrayLength,
                           RHIImageLayout const layoutNew);

        RHISyncPrimitive* getRenderingCompleteSemaphore();
        RHICommandListState getState() const;
        RHIQueue* getQueue() const;

    private:
        std::shared_ptr<RHISyncPrimitive> m_renderingCompleteBinaySemaphore;
        std::shared_ptr<RHISyncPrimitive> m_renderingCompleteTimelineSemaphore;

        std::atomic<RHICommandListState> m_state = RHICommandListState::Idle;
        RHIQueue* m_queue                        = nullptr;
        RHIResource m_cmdList                    = {};
    };

} // namespace worse