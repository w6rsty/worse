#include "RHISyncPrimitive.hpp"
#include "RHIDevice.hpp"

namespace worse
{

    namespace fence
    {
        void create(RHINativeHandle& resource)
        {
            VkFenceCreateInfo infoFence = {};
            infoFence.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

            VkFence fence = VK_NULL_HANDLE;
            WS_ASSERT_VK(
                vkCreateFence(RHIContext::device, &infoFence, nullptr, &fence));
            resource = RHINativeHandle{fence, RHINativeHandleType::Fence};
        }

        void wait(RHINativeHandle& resource, std::uint64_t const timeoutNs)
        {
            VkFence fence = resource.asValue<VkFence>();
            WS_ASSERT_VK(vkWaitForFences(RHIContext::device,
                                         1,
                                         &fence,
                                         VK_TRUE,
                                         timeoutNs));
        }

        bool isSignaled(RHINativeHandle& resource)
        {
            return vkGetFenceStatus(RHIContext::device,
                                    resource.asValue<VkFence>()) == VK_SUCCESS;
        }

        void reset(RHINativeHandle& resource)
        {
            VkFence fence = resource.asValue<VkFence>();
            WS_ASSERT_VK(vkResetFences(RHIContext::device, 1, &fence));
        }
    } // namespace fence

    namespace semaphore
    {
        void create(RHISyncPrimitiveType type, RHINativeHandle& resource)
        {
            // clang-format off
            VkSemaphoreTypeCreateInfo infoSemType = {};
            infoSemType.sType                     = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
            infoSemType.semaphoreType             = VK_SEMAPHORE_TYPE_TIMELINE;
            infoSemType.initialValue              = 0;
            // clang-format on

            // clang-format off
            VkSemaphoreCreateInfo infoSem = {};
            infoSem.sType                = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            infoSem.pNext                = type == RHISyncPrimitiveType::TimelineSemaphore
                                           ? &infoSemType
                                           : nullptr;
            // clang-format on

            VkSemaphore semaphore = VK_NULL_HANDLE;
            WS_ASSERT_VK(vkCreateSemaphore(RHIContext::device,
                                           &infoSem,
                                           nullptr,
                                           &semaphore));
            resource =
                RHINativeHandle{semaphore, RHINativeHandleType::Semaphore};
        }

        void wait(RHINativeHandle& resource, std::uint64_t const value,
                  std::uint64_t const timeoutNs)
        {
            VkSemaphore semaphore = resource.asValue<VkSemaphore>();

            // clang-format off
            VkSemaphoreWaitInfo infoWait = {};
            infoWait.sType               = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
            infoWait.semaphoreCount      = 1;
            infoWait.pSemaphores         = &semaphore;
            infoWait.pValues             = &value;
            // clang-format on
            WS_ASSERT_VK(
                vkWaitSemaphores(RHIContext::device, &infoWait, timeoutNs));
        }

        void signal(RHINativeHandle& resource, std::uint64_t const value)
        {
            // clang-format off
            VkSemaphoreSignalInfo infoSignal = {};
            infoSignal.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
            infoSignal.semaphore             = resource.asValue<VkSemaphore>();
            infoSignal.value                 = value;
            // clang-format on
            WS_ASSERT_VK(vkSignalSemaphore(RHIContext::device, &infoSignal));
        }

        std::uint64_t getValue(RHINativeHandle& resource)
        {
            std::uint64_t value = 0;
            WS_ASSERT_VK(
                vkGetSemaphoreCounterValue(RHIContext::device,
                                           resource.asValue<VkSemaphore>(),
                                           &value));
            return value;
        }
    } // namespace semaphore

    RHISyncPrimitive::RHISyncPrimitive(RHISyncPrimitiveType const type,
                                       std::string_view name)
        : RHIResource(name)
    {
        m_type = type;

        if (type == RHISyncPrimitiveType::Fence)
        {
            fence::create(m_handle);
            RHIDevice::setResourceName(m_handle, name);
        }
        else
        {
            semaphore::create(type, m_handle);
            RHIDevice::setResourceName(m_handle, name);
        }
    }

    RHISyncPrimitive::~RHISyncPrimitive()
    {
        RHIDevice::deletionQueueAdd(m_handle);
        m_handle = {};
    }

    void RHISyncPrimitive::wait(std::uint64_t const timeoutNs)
    {
        WS_ASSERT((m_type == RHISyncPrimitiveType::Fence) ||
                  (m_type == RHISyncPrimitiveType::TimelineSemaphore));

        if (m_type == RHISyncPrimitiveType::Fence)
        {
            fence::wait(m_handle, timeoutNs);
        }
        else
        {
            semaphore::wait(m_handle, m_value, timeoutNs);
        }
    }

    void RHISyncPrimitive::signal(std::uint64_t const value)
    {
        WS_ASSERT(m_type == RHISyncPrimitiveType::TimelineSemaphore);

        semaphore::signal(m_handle, value);
    }

    bool RHISyncPrimitive::isSignaled()
    {
        WS_ASSERT((m_type == RHISyncPrimitiveType::Fence) ||
                  (m_type == RHISyncPrimitiveType::TimelineSemaphore));

        if (m_type == RHISyncPrimitiveType::Fence)
        {
            return fence::isSignaled(m_handle);
        }
        else
        {
            return semaphore::getValue(m_handle) == m_value;
        }
    }

    void RHISyncPrimitive::reset()
    {
        WS_ASSERT(m_type == RHISyncPrimitiveType::Fence);

        fence::reset(m_handle);
    }

} // namespace worse