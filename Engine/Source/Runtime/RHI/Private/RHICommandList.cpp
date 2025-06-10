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

} // namespace worse