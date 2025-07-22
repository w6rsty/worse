#pragma once
#include "RHIResource.hpp"

#include <cstdint>

namespace worse
{

    class RHISyncPrimitive : public RHIResource
    {
    public:
        RHISyncPrimitive(RHISyncPrimitiveType const type,
                         std::string_view name);
        ~RHISyncPrimitive();

        void wait(u64 const timeoutNs);
        void signal(u64 const value);
        bool isSignaled();
        void reset();

        u64 getNextSignalValue();
        RHINativeHandle getHandle() const;
        void setBelongingCmdList(RHICommandList* cmdList);
        RHICommandList* getBelongingCmdList() const;

        bool hasBeenWaitedFor = false;

    private:
        RHISyncPrimitiveType m_type        = RHISyncPrimitiveType::Max;
        u64 m_value                        = 0; // timeline counter
        RHICommandList* m_belongingCmdList = nullptr;
        RHINativeHandle m_handle;
    };

} // namespace worse