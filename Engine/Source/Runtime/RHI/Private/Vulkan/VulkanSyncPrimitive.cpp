#include "RHISyncPrimitive.hpp"
#include "RHIDevice.hpp"

namespace worse
{

    namespace fence
    {
        void create(RHIResource& resource)
        {
            VkFenceCreateInfo infoFence = {};
            infoFence.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

            VkFence fence = VK_NULL_HANDLE;
            WS_ASSERT_VK(
                vkCreateFence(RHIContext::device, &infoFence, nullptr, &fence));
            resource = RHIResource{fence, RHIResourceType::Fence};
        }

        void wait(RHIResource& resource, std::uint64_t const timeoutNs)
        {
            VkFence fence = resource.asValue<VkFence>();
            WS_ASSERT_VK(vkWaitForFences(RHIContext::device,
                                         1,
                                         &fence,
                                         VK_TRUE,
                                         timeoutNs));
        }

        bool isSignaled(RHIResource& resource)
        {
            return vkGetFenceStatus(RHIContext::device,
                                    resource.asValue<VkFence>()) == VK_SUCCESS;
        }

        void reset(RHIResource& resource)
        {
            VkFence fence = resource.asValue<VkFence>();
            WS_ASSERT_VK(vkResetFences(RHIContext::device, 1, &fence));
        }
    } // namespace fence

    namespace semaphore
    {
        void create(RHISyncPrimitiveType type, RHIResource& resource)
        {
            VkSemaphoreTypeCreateInfo infoSemType = {};
            infoSemType.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
            infoSemType.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
            infoSemType.initialValue  = 0;

            VkSemaphoreCreateInfo infoSem = {};
            infoSem.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            infoSem.pNext = type == RHISyncPrimitiveType::TimelineSemaphore
                                ? &infoSemType
                                : nullptr;

            VkSemaphore semaphore = VK_NULL_HANDLE;
            WS_ASSERT_VK(vkCreateSemaphore(RHIContext::device,
                                           &infoSem,
                                           nullptr,
                                           &semaphore));
            resource = RHIResource{semaphore, RHIResourceType::Semaphore};
        }

        void wait(RHIResource& resource, std::uint64_t const value,
                  std::uint64_t const timeoutNs)
        {
            VkSemaphore semaphore = resource.asValue<VkSemaphore>();

            VkSemaphoreWaitInfo infoWait = {};
            infoWait.sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
            infoWait.semaphoreCount = 1;
            infoWait.pSemaphores    = &semaphore;
            infoWait.pValues        = &value;
            WS_ASSERT_VK(
                vkWaitSemaphores(RHIContext::device, &infoWait, timeoutNs));
        }

        void signal(RHIResource& resource, std::uint64_t const value)
        {
            VkSemaphoreSignalInfo infoSignal = {};
            infoSignal.sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
            infoSignal.semaphore = resource.asValue<VkSemaphore>();
            infoSignal.value     = value;
            WS_ASSERT_VK(vkSignalSemaphore(RHIContext::device, &infoSignal));
        }

        std::uint64_t getValue(RHIResource& resource)
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
                                       char const* name)
    {
        m_type = type;

        if (type == RHISyncPrimitiveType::Fence)
        {
            fence::create(m_rhiResource);
            RHIDevice::setResourceName(m_rhiResource, name);
        }
        else
        {
            semaphore::create(type, m_rhiResource);
            RHIDevice::setResourceName(m_rhiResource, name);
        }
    }

    RHISyncPrimitive::~RHISyncPrimitive()
    {
        if (!m_rhiResource)
        {
            return;
        }

        RHIDevice::deletionQueueAdd(m_rhiResource);
        m_rhiResource.reset();
    }

    void RHISyncPrimitive::wait(std::uint64_t const timeoutNs)
    {
        WS_ASSERT((m_type == RHISyncPrimitiveType::Fence) ||
                  (m_type == RHISyncPrimitiveType::TimelineSemaphore));

        if (m_type == RHISyncPrimitiveType::Fence)
        {
            fence::wait(m_rhiResource, timeoutNs);
        }
        else
        {
            semaphore::wait(m_rhiResource, m_value, timeoutNs);
        }
    }

    void RHISyncPrimitive::signal(std::uint64_t const value)
    {
        WS_ASSERT(m_type == RHISyncPrimitiveType::TimelineSemaphore);

        semaphore::signal(m_rhiResource, value);
    }

    bool RHISyncPrimitive::isSignaled()
    {
        WS_ASSERT((m_type == RHISyncPrimitiveType::Fence) ||
                  (m_type == RHISyncPrimitiveType::TimelineSemaphore));

        if (m_type == RHISyncPrimitiveType::Fence)
        {
            return fence::isSignaled(m_rhiResource);
        }
        else
        {
            return semaphore::getValue(m_rhiResource) == m_value;
        }
    }

    void RHISyncPrimitive::reset()
    {
        WS_ASSERT(m_type == RHISyncPrimitiveType::Fence);

        fence::reset(m_rhiResource);
    }

} // namespace worse