#include "RHISyncPrimitive.hpp"

namespace worse
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

} // namespace worse