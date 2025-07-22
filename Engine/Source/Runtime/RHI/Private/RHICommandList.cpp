#include "RHICommandList.hpp"
#include "RHISyncPrimitive.hpp" // IWYU pragma: keep

namespace worse
{

    void RHICommandList::waitForExecution()
    {
        WS_ASSERT(m_state == RHICommandListState::Submitted);

        u64 const timeoutNs = 60'000'000'000; // 60s
        m_renderingCompleteTimelineSemaphore->wait(timeoutNs);
        m_state = RHICommandListState::Idle;
    }

} // namespace worse