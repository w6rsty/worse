#include "Window.hpp"
#include "Log.hpp"
#include "Event.hpp"
#include "RHISwapchain.hpp"
#include "RHIResource.hpp"
#include "RHIDevice.hpp"
#include "RHIQueue.hpp"

#include "SDL3/SDL_vulkan.h"

namespace worse
{
    namespace
    {
        constexpr RHIFormat formatSDR = RHIFormat::R8G8B8A8Unorm;

        VkSurfaceFormatKHR surfaceFormat;

        VkSurfaceCapabilitiesKHR
        getSurfaceCapabilities(VkSurfaceKHR const surface)
        {
            VkSurfaceCapabilitiesKHR capabilities = {};
            WS_ASSERT_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                RHIContext::physicalDevice,
                surface,
                &capabilities));
            return capabilities;
        }

        VkSurfaceFormatKHR getSurfaceFormat(VkSurfaceKHR const surface)
        {
            // TODO: support HDR format
            std::uint32_t surfaceFormatCount = 0;
            WS_ASSERT_VK(
                vkGetPhysicalDeviceSurfaceFormatsKHR(RHIContext::physicalDevice,
                                                     surface,
                                                     &surfaceFormatCount,
                                                     nullptr));
            std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
            WS_ASSERT_VK(
                vkGetPhysicalDeviceSurfaceFormatsKHR(RHIContext::physicalDevice,
                                                     surface,
                                                     &surfaceFormatCount,
                                                     surfaceFormats.data()));

            auto formatSelector = [](VkFormat format) -> std::uint32_t
            {
                switch (format)
                {
                case VK_FORMAT_R8G8B8A8_SRGB:
                    return 90;
                case VK_FORMAT_B8G8R8A8_SRGB:
                    return 90;
                case VK_FORMAT_R8G8B8A8_UNORM:
                    return 100;
                case VK_FORMAT_B8G8R8A8_UNORM:
                    return 100;
                default:
                    return 0;
                }
            };

            auto bestFormat = std::ranges::max_element(
                surfaceFormats,
                [&](VkSurfaceFormatKHR const& lhs,
                    VkSurfaceFormatKHR const& rhs) -> bool
                {
                    return formatSelector(lhs.format) <
                           formatSelector(rhs.format);
                });

            return *bestFormat;
        }

        VkCompositeAlphaFlagBitsKHR
        getCompositeAlpha(VkSurfaceKHR const surface)
        {
            std::array flagBits = {VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                                   VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
                                   VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
                                   VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR};

            VkSurfaceCapabilitiesKHR capabilities = {};
            WS_ASSERT_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                RHIContext::physicalDevice,
                surface,
                &capabilities));

            for (auto flag : flagBits)
            {
                if (capabilities.supportedCompositeAlpha & flag)
                {
                    return flag;
                }
            }

            return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // default
        }

        VkPresentModeKHR getPresentMode(VkSurfaceKHR const surface,
                                        RHIPresentMode const presentMode)
        {
            VkPresentModeKHR defaultPresentMode = VK_PRESENT_MODE_FIFO_KHR;
            if (presentMode == RHIPresentMode::Immediate)
            {
                defaultPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
            else if (presentMode == RHIPresentMode::Mailbox)
            {
                defaultPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            }

            std::uint32_t presentModeCount = 0;
            WS_ASSERT_VK(vkGetPhysicalDeviceSurfacePresentModesKHR(
                RHIContext::physicalDevice,
                surface,
                &presentModeCount,
                nullptr));
            std::vector<VkPresentModeKHR> presentModes(presentModeCount);
            WS_ASSERT_VK(vkGetPhysicalDeviceSurfacePresentModesKHR(
                RHIContext::physicalDevice,
                surface,
                &presentModeCount,
                presentModes.data()));

            for (auto mode : presentModes)
            {
                if (mode == defaultPresentMode)
                {
                    return mode;
                }
            }

            WS_LOG_WARN(
                "Swapchain",
                "Requested present mode not supported, falling back to FIFO");
            return VK_PRESENT_MODE_FIFO_KHR;
        }

    } // namespace

    RHISwapchain::RHISwapchain(void* sdlWindow, std::uint32_t const width,
                               std::uint32_t const height,
                               RHIPresentMode const presentMode,
                               std::string_view name)
        : RHIResource(name)
    {
        m_format      = formatSDR;
        m_width       = width;
        m_height      = height;
        m_presentMode = presentMode;
        m_sdlWindow   = sdlWindow;
        m_presentMode = presentMode;

        // create surface
        {
            VkSurfaceKHR surface = VK_NULL_HANDLE;

            if (!SDL_Vulkan_CreateSurface(static_cast<SDL_Window*>(m_sdlWindow),
                                          RHIContext::instance,
                                          nullptr,
                                          &surface))
            {
                WS_LOG_ERROR("Swapchain",
                             "Failed to create surface: {}",
                             SDL_GetError());
            }

            VkBool32 presentSupport{VK_FALSE};
            WS_ASSERT_VK(vkGetPhysicalDeviceSurfaceSupportKHR(
                RHIContext::physicalDevice,
                RHIDevice::getQueueIndex(RHIQueueType::Graphics),
                surface,
                &presentSupport));
            WS_ASSERT_MSG(presentSupport, "Surface does not support present");

            m_surface = RHINativeHandle{surface, RHINativeHandleType::Surface};
        }

        RHISwapchain::create();

        EventBus::subscribe(EventType::WindowResized,
                            [this](Event const&)
                            {
                                RHISwapchain::resizeFitWindow();
                            });
    }

    RHISwapchain::~RHISwapchain()
    {
        for (RHINativeHandle& imageView : m_rtvs)
        {

            RHIDevice::deletionQueueAdd(imageView);
            imageView = {};
        }

        if (m_swapchain)
        {
            vkDestroySwapchainKHR(RHIContext::device,
                                  m_swapchain.asValue<VkSwapchainKHR>(),
                                  nullptr);
        }

        if (m_surface)
        {
            vkDestroySurfaceKHR(RHIContext::instance,
                                m_surface.asValue<VkSurfaceKHR>(),
                                nullptr);
        }
    }

    void RHISwapchain::resize(std::uint32_t width, std::uint32_t height)
    {
        if (width == m_width && height == m_height)
        {
            return;
        }

        m_width  = width;
        m_height = height;

        // recreate swapchain
        create();

        WS_LOG_INFO("Swapchain", "Resized to {}x{}", m_width, m_height);
    }

    void RHISwapchain::resizeFitWindow()
    {
        resize(Window::getWidth(), Window::getHeight());
    }

    void RHISwapchain::acquireNextImage()
    {
        // clang-format off
        if (Window::isMinimized())
        {
            return;
        }

        static std::uint64_t semaphoreIndex = 0;
        RHISyncPrimitive* semaphoreSignal = m_imageAcquireSemaphores[semaphoreIndex].get();

        if (RHICommandList* cmdList = semaphoreSignal->getBelongingCmdList())
        {
            if (cmdList->getState() == RHICommandListState::Submitted)
            {
                cmdList->waitForExecution();
            }
            WS_ASSERT(cmdList->getState() == RHICommandListState::Idle);
        }

        std::uint32_t retryCount          = 0;
        std::uint32_t const maxRetryCount = 10;

        while (retryCount < maxRetryCount)
        {
            VkResult result = vkAcquireNextImageKHR(
                RHIContext::device,
                m_swapchain.asValue<VkSwapchainKHR>(),
                16'000'000, // 16 ms,
                semaphoreSignal->getHandle().asValue<VkSemaphore>(),
                VK_NULL_HANDLE,
                &m_imageIndex);

            if (result == VK_SUCCESS)
            {
                m_imageAcquireSemaphores[m_imageIndex] = m_imageAcquireSemaphores[semaphoreIndex];
                semaphoreIndex = (semaphoreIndex + 1) % m_imageAcquireSemaphores.size();
                return;
            }
            else if (result == VK_NOT_READY)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
                ++retryCount;
            }
            else
            {
                WS_ASSERT_VK(result);
            }
        }
        // clang-format on
    }

    void RHISwapchain::present(RHICommandList* cmdList)
    {
        if (Window::isMinimized())
        {
            return;
        }

        cmdList->getQueue()->present(m_swapchain,
                                     m_imageIndex,
                                     cmdList->getRenderingCompleteSemaphore());

        if (m_isDirty)
        {
            create();
            m_isDirty = false;
        }
    }

    void RHISwapchain::create()
    {
        WS_ASSERT(m_sdlWindow != nullptr);
        WS_ASSERT(m_surface.isValid());

        VkSurfaceCapabilitiesKHR cap =
            getSurfaceCapabilities(m_surface.asValue<VkSurfaceKHR>());

        if ((cap.currentExtent.width == 0) || (cap.currentExtent.height == 0))
        {
            WS_LOG_WARN("Swapchain", "Window is minimized, skipping creation");
            return;
        }

        surfaceFormat = getSurfaceFormat(m_surface.asValue<VkSurfaceKHR>());

        m_width  = std::clamp(m_width,
                             cap.minImageExtent.width,
                             cap.maxImageExtent.width);
        m_height = std::clamp(m_height,
                              cap.minImageExtent.height,
                              cap.maxImageExtent.height);

        // swapchain creation
        {
            // clang-format off
            VkSwapchainCreateInfoKHR infoSwapchain = {};
            infoSwapchain.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            infoSwapchain.surface                  = m_surface.asValue<VkSurfaceKHR>();
            infoSwapchain.minImageCount            = s_bufferCount;
            infoSwapchain.imageFormat              = surfaceFormat.format;
            infoSwapchain.imageColorSpace          = surfaceFormat.colorSpace;
            infoSwapchain.imageExtent              = {m_width, m_height};
            infoSwapchain.imageArrayLayers         = 1;
            infoSwapchain.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            infoSwapchain.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
            infoSwapchain.preTransform             = cap.currentTransform;
            infoSwapchain.compositeAlpha           = getCompositeAlpha(m_surface.asValue<VkSurfaceKHR>());
            infoSwapchain.presentMode              = getPresentMode(m_surface.asValue<VkSurfaceKHR>(), m_presentMode);
            infoSwapchain.clipped                  = VK_TRUE;
            infoSwapchain.oldSwapchain             = m_swapchain.asValue<VkSwapchainKHR>();
            // clang-format on

            WS_ASSERT_VK(vkCreateSwapchainKHR(
                RHIContext::device,
                &infoSwapchain,
                nullptr,
                reinterpret_cast<VkSwapchainKHR*>(&m_swapchain)));

            if (infoSwapchain.oldSwapchain != VK_NULL_HANDLE)
            {
                RHIDevice::queueWaitAll();
                vkDestroySwapchainKHR(RHIContext::device,
                                      infoSwapchain.oldSwapchain,
                                      nullptr);
            }
        }

        // get images
        {
            std::uint32_t imageCount = 0;
            WS_ASSERT_VK(
                vkGetSwapchainImagesKHR(RHIContext::device,
                                        m_swapchain.asValue<VkSwapchainKHR>(),
                                        &imageCount,
                                        nullptr));
            std::vector<VkImage> swapchainImages(imageCount);
            WS_ASSERT_VK(
                vkGetSwapchainImagesKHR(RHIContext::device,
                                        m_swapchain.asValue<VkSwapchainKHR>(),
                                        &imageCount,
                                        swapchainImages.data()));
            for (std::uint32_t i = 0;
                 i < static_cast<std::uint32_t>(swapchainImages.size());
                 ++i)
            {
                m_rts[i] = RHINativeHandle{swapchainImages[i],
                                           RHINativeHandleType::Image};
            }
        }

        // create image views
        {
            for (std::uint32_t i = 0; i < s_bufferCount; ++i)
            {
                // destroy old one
                if (m_rtvs[i])
                {
                    RHIDevice::deletionQueueAdd(m_rtvs[i]);
                }

                // clang-format off
                VkImageViewCreateInfo infoIV = {};
                infoIV.sType                 = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                infoIV.image                 = m_rts[i].asValue<VkImage>();
                infoIV.viewType              = VK_IMAGE_VIEW_TYPE_2D;
                infoIV.format                = surfaceFormat.format;
                infoIV.subresourceRange      = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
                infoIV.components            = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
                // clang-format on

                VkImageView imageView = VK_NULL_HANDLE;
                WS_ASSERT_VK(vkCreateImageView(RHIContext::device,
                                               &infoIV,
                                               nullptr,
                                               &imageView));
                m_rtvs[i] =
                    RHINativeHandle{imageView, RHINativeHandleType::ImageView};
            }
        }

        // image barrier
        {
            // TODO: Submit to immediate queue
        }

        // create sync primtives
        for (std::uint32_t i = 0;
             i < static_cast<std::uint32_t>(m_imageAcquireSemaphores.size());
             ++i)
        {
            m_imageAcquireSemaphores[i] = std::make_shared<RHISyncPrimitive>(
                RHISyncPrimitiveType::BinarySemaphore,
                std::format("image_acquire_{}", i).c_str());
        }

        WS_LOG_INFO("Swapchain",
                    "Created (Size: {}x{}, Format: {}, VSync: {})",
                    m_width,
                    m_height,
                    rhiFormatToString(m_format),
                    m_presentMode == RHIPresentMode::Immediate ? "off" : "on");

        // reset state
        m_imageIndex = 0;
    }

} // namespace worse