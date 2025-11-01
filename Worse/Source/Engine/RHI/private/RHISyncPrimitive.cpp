#include "RHISyncPrimitive.hpp"

namespace Worse
{
    u64 RHISyncPrimitive::getNextSignalValue()
    {
        return ++m_value;
    }

    RHINativeHandle RHISyncPrimitive::getHandle() const
    {
        return m_handle;
    }

    void RHISyncPrimitive::setBelongingCmdList(RHICommandList* cmdList)
    {
        m_belongingCmdList = cmdList;
    }

    RHICommandList* RHISyncPrimitive::getBelongingCmdList() const
    {
        return m_belongingCmdList;
    }

} // namespace Worse