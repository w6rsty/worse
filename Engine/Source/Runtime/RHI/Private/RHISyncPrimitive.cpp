#include "RHISyncPrimitive.hpp"

namespace worse
{
    std::uint64_t RHISyncPrimitive::getNextSignalValue()
    {
        return ++m_value;
    }

    RHIResource RHISyncPrimitive::getRHIResource() const
    {
        return m_rhiResource;
    }

    void RHISyncPrimitive::setCmdList(RHICommandList* cmdList)
    {
        m_cmdList = cmdList;
    }

    RHICommandList* RHISyncPrimitive::getCmdList() const
    {
        return m_cmdList;
    }

} // namespace worse