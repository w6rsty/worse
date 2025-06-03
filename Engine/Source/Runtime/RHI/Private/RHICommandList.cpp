#include "RHICommandList.hpp"
#include "RHISyncPrimitive.hpp"

namespace worse
{

    void RHICommandList::waitForExecution()
    {
        WS_ASSERT(m_state == RHICommandListState::Submitted);

        std::uint64_t const timeoutNs = 60'000'000'000; // 60s
        m_renderingCompleteTimelineSemaphore->wait(timeoutNs);
        m_state = RHICommandListState::Idle;
    }

    RHISyncPrimitive* RHICommandList::getRenderingCompleteSemaphore()
    {
        return m_renderingCompleteBinaySemaphore.get();
    }

    RHICommandListState RHICommandList::getState() const
    {
        return m_state;
    }

    RHIQueue* RHICommandList::getQueue() const
    {
        return m_submissionQueue;
    }

} // namespace worse