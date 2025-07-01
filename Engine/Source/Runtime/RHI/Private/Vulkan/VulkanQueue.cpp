#include "RHICommandList.hpp"
#include "RHIQueue.hpp"
#include "RHIDevice.hpp"

#include <mutex>

namespace worse
{

    namespace
    {
        EnumArray<RHIQueueType, std::mutex> mtxes;

        std::mutex const& getMutex(RHIQueueType const type)
        {
            return mtxes[type];
        }
    } // namespace

    RHIQueue::RHIQueue(RHIQueueType type, std::string_view name)
        : RHIResource(name)
    {
        m_type = type;
        // command pool
        {
            // clang-format off
            VkCommandPoolCreateInfo infoCmdPool = {};
            infoCmdPool.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            infoCmdPool.queueFamilyIndex        = RHIDevice::getQueueIndex(type);
            infoCmdPool.flags                   = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
                                                  VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            // clang-format on

            VkCommandPool cmdPool = VK_NULL_HANDLE;
            WS_ASSERT_VK(vkCreateCommandPool(RHIContext::device,
                                             &infoCmdPool,
                                             nullptr,
                                             &cmdPool));
            m_handle =
                RHINativeHandle{cmdPool, RHINativeHandleType::CommandPool};
            RHIDevice::setResourceName(m_handle, name);
        }

        // command lists
        {
            for (std::uint32_t i = 0;
                 i < static_cast<std::uint32_t>(m_cmdLists.size());
                 ++i)
            {
                m_cmdLists[i] = std::make_shared<RHICommandList>(
                    this,
                    m_handle,
                    std::format("cmd_list_{}", i).c_str());
            }
        }
    }

    RHIQueue::~RHIQueue()
    {
        wait();

        // Clear command lists before destroying command pool
        // This ensures semaphores are destroyed before the device
        for (auto& cmdList : m_cmdLists)
        {
            cmdList.reset();
        }

        vkDestroyCommandPool(RHIContext::device,
                             m_handle.asValue<VkCommandPool>(),
                             nullptr);
    }

    void RHIQueue::wait()
    {
        // TODO: lock

        WS_ASSERT_VK(vkQueueWaitIdle(
            RHIDevice::getQueueHandle(m_type).asValue<VkQueue>()));
    }

    void RHIQueue::submit(void* cmdBuffer, std::uint32_t const waitFlags,
                          RHISyncPrimitive* semaphoreWait,
                          RHISyncPrimitive* semaphoreSignal,
                          RHISyncPrimitive* semaphoreTimeline)
    {
        // TODO: lock

        // clang-format off
        VkSemaphoreSubmitInfo semaphoresWait[1] = {};
        {
            semaphoresWait[0].sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            semaphoresWait[0].semaphore = semaphoreWait ? semaphoreWait->getHandle().asValue<VkSemaphore>() : nullptr;
            semaphoresWait[0].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            semaphoresWait[0].value     = 0;
        }

        VkSemaphoreSubmitInfo semaphoresSignal[2] = {};
        {
            // binary
            semaphoresSignal[0].sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            semaphoresSignal[0].semaphore = semaphoreSignal->getHandle().asValue<VkSemaphore>();
            semaphoresSignal[0].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            semaphoresSignal[0].value     = 0;

            // timeline
            semaphoresSignal[1].sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            semaphoresSignal[1].semaphore = semaphoreTimeline->getHandle().asValue<VkSemaphore>();
            semaphoresSignal[1].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            semaphoresSignal[1].value     = semaphoreTimeline->getNextSignalValue();
        }
        // clang-format on

        VkCommandBufferSubmitInfo infoCmdBuffer = {};
        infoCmdBuffer.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        infoCmdBuffer.commandBuffer =
            *reinterpret_cast<VkCommandBuffer*>(&cmdBuffer);

        VkSubmitInfo2 infoSubmit            = {};
        infoSubmit.sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        infoSubmit.waitSemaphoreInfoCount   = semaphoreWait ? 1 : 0;
        infoSubmit.pWaitSemaphoreInfos      = semaphoresWait;
        infoSubmit.signalSemaphoreInfoCount = 2;
        infoSubmit.pSignalSemaphoreInfos    = semaphoresSignal;
        infoSubmit.commandBufferInfoCount   = 1;
        infoSubmit.pCommandBufferInfos      = &infoCmdBuffer;

        WS_ASSERT_VK(vkQueueSubmit2KHR(
            RHIDevice::getQueueHandle(m_type).asValue<VkQueue>(),
            1,
            &infoSubmit,
            nullptr));
    }

    void RHIQueue::present(RHINativeHandle swapchain,
                           std::uint32_t const imageIndex,
                           RHISyncPrimitive* semaphoreWait)
    {
        // TODO: lock

        VkSemaphore semaphoresWait[1] = {};
        semaphoresWait[0] = semaphoreWait->getHandle().asValue<VkSemaphore>();

        VkSwapchainKHR vkSwapchain = swapchain.asValue<VkSwapchainKHR>();

        VkPresentInfoKHR infoPresent   = {};
        infoPresent.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        infoPresent.waitSemaphoreCount = 1;
        infoPresent.pWaitSemaphores    = semaphoresWait;
        infoPresent.swapchainCount     = 1;
        infoPresent.pSwapchains        = &vkSwapchain;
        infoPresent.pImageIndices      = &imageIndex;

        WS_ASSERT_VK(vkQueuePresentKHR(
            RHIDevice::getQueueHandle(m_type).asValue<VkQueue>(),
            &infoPresent));
    }

    RHICommandList* RHIQueue::nextCommandList()
    {
        m_index       = (m_index + 1) % m_cmdLists.size();
        auto& cmdList = m_cmdLists[m_index.load()];

        if (cmdList->getState() == RHICommandListState::Recording)
        {
            cmdList->submit(nullptr);
        }

        if (cmdList->getState() == RHICommandListState::Submitted)
        {
            cmdList->waitForExecution();
        }

        WS_ASSERT(cmdList->getState() == RHICommandListState::Idle);

        return cmdList.get();
    }

} // namespace worse