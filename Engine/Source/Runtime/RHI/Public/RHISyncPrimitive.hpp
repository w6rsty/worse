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

        void wait(std::uint64_t const timeoutNs);
        void signal(std::uint64_t const value);
        bool isSignaled();
        void reset();

        std::uint64_t getNextSignalValue();
        RHINativeHandle getHandle() const;
        void setBelongingCmdList(RHICommandList* cmdList);
        RHICommandList* getBelongingCmdList() const;

        bool hasBeenWaitedFor = false;

    private:
        RHISyncPrimitiveType m_type        = RHISyncPrimitiveType::Max;
        std::uint64_t m_value              = 0; // timeline counter
        RHICommandList* m_belongingCmdList = nullptr;
        RHINativeHandle m_handle;
    };

} // namespace worse