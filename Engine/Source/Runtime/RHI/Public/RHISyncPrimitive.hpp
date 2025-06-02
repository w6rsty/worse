#pragma once
#include "RHIDefinitions.hpp"
#include "RHIResource.hpp"

#include <cstdint>

namespace worse
{
    // Forward declaration to avoid circular include
    class RHICommandList;

    class RHISyncPrimitive
    {
    public:
        RHISyncPrimitive(RHISyncPrimitiveType const type, char const* name);
        ~RHISyncPrimitive();

        void wait(std::uint64_t const timeoutNs);
        void signal(std::uint64_t const value);
        bool isSignaled();
        void reset();

        std::uint64_t getNextSignalValue();
        RHIResource getRHIResource() const;
        void setCmdList(RHICommandList* cmdList);
        RHICommandList* getCmdList() const;

        bool hasBeenWaitedFor = false;

    private:
        RHICommandList* m_cmdList   = nullptr;
        RHISyncPrimitiveType m_type = RHISyncPrimitiveType::Max;
        std::uint64_t m_value       = 0;
        RHIResource m_rhiResource;
    };

} // namespace worse