#include "RHISyncPrimitive.hpp"

namespace worse
{
    std::uint64_t RHISyncPrimitive::getNextSignalValue()
    {
        return ++m_value;
    }

    RHINativeHandle RHISyncPrimitive::getRHIResource() const
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