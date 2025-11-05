#pragma once
#include "RHIResource.hpp"

namespace Worse
{

    class RHISyncPrimitive : public RHIResource
    {
    public:
        RHISyncPrimitive(RHISyncPrimitiveType const type,
                         std::string_view name);
        ~RHISyncPrimitive();

        void wait(ULong const timeoutNs);
        void signal(ULong const value);
        Bool isSignaled();
        void reset();

        ULong getNextSignalValue();
        RHINativeHandle getHandle() const;
        void setBelongingCmdList(RHICommandList* cmdList);
        RHICommandList* getBelongingCmdList() const;

        Bool hasBeenWaitedFor = false;

    private:
        RHISyncPrimitiveType m_type        = RHISyncPrimitiveType::Max;
        ULong m_value                        = 0; // timeline counter
        RHICommandList* m_belongingCmdList = nullptr;
        RHINativeHandle m_handle;
    };

} // namespace Worse