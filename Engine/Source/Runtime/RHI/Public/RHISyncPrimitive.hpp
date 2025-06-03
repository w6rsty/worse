#pragma once
#include "RHIDefinitions.hpp"
#include "RHIResource.hpp"

#include <cstdint>

namespace worse
{
    // Forward declaration to avoid circular include
    class RHICommandList;

    class RHISyncPrimitive : public RHIResource
    {
    public:
        RHISyncPrimitive(RHISyncPrimitiveType const type, std::string_view name);
        ~RHISyncPrimitive();

        void wait(std::uint64_t const timeoutNs);
        void signal(std::uint64_t const value);
        bool isSignaled();
        void reset();

        std::uint64_t getNextSignalValue();
        RHINativeHandle getRHIResource() const;
        void setBelongingCmdList(RHICommandList* cmdList);
        RHICommandList* getBelongingCmdList() const;

        bool hasBeenWaitedFor = false;

    private:
        RHISyncPrimitiveType m_type = RHISyncPrimitiveType::Max;
        std::uint64_t m_value       = 0; // timeline counter
        RHINativeHandle m_handle;
        RHICommandList* m_belongingCmdList = nullptr;
    };

} // namespace worse