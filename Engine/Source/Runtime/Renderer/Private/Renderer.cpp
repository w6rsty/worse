#include "RHIDefinitions.hpp"
#include "Renderer.hpp"
#include "Window.hpp"
#include "RHIDevice.hpp"
#include "RHIViewport.hpp"
#include "RHISwapchain.hpp"
#include "RHIQueue.hpp"
#include "Math/Vector2.hpp"

#include <memory>

namespace worse
{
    namespace config
    {
        bool enableVsync = true;
    }

    namespace
    {
        math::Vector2 resolutionRender = math::Vector2{0, 0};
        math::Vector2 resolutionOutput = math::Vector2{0, 0};
        RHIViewport viewport           = RHIViewport(0, 0, 0, 0);

        std::shared_ptr<RHISwapchain> swapchain = nullptr;
        std::uint32_t swapchainBufferCount      = 2;
        RHICommandList* m_cmdListCurrent        = nullptr;

    } // namespace

    void Renderer::initialize()
    {
        RHIDevice::initialize();

        // resolution
        {
            std::uint32_t width  = Window::getWidth();
            std::uint32_t height = Window::getHeight();

            // output resolution

            // render resolution

            Renderer::setViewport(static_cast<float>(width),
                                  static_cast<float>(height));
        }

        // swapchain
        {
            swapchain = std::make_shared<RHISwapchain>(
                Window::getHandleSDL(),
                Window::getWidth(),
                Window::getHeight(),
                config::enableVsync ? RHIPresentMode::FIFO
                                    : RHIPresentMode::Immediate,
                swapchainBufferCount,
                "swapchain");
        }
    }

    void Renderer::shutdown()
    {
        RHIDevice::queueWaitAll();

        {
            swapchain = nullptr;
        }

        RHIDevice::destroy();
    }

    void Renderer::tick()
    {
        // clang-format off

        swapchain->acquireNextImage(); // signal image acquire semaphore(swapchain)

        RHIQueue* graphicsQueue = RHIDevice::getQueue(RHIQueueType::Graphics);
        m_cmdListCurrent        = graphicsQueue->nextCommandList();
        m_cmdListCurrent->begin(); // begin command list

        RHIDevice::deletionQueueFlush();

        Renderer::submitAndPresent(); // [Sumbit] wait image acquire semaphore(swapchain)
                                      //          signal rendering semaphore(CommandList)
                                      // [Present] wait rendering semaphore(CommandList)
        // clang-format on
    }

    RHISwapchain* Renderer::getSwapchain()
    {
        return swapchain.get();
    }

    void Renderer::submitAndPresent()
    {
        if (m_cmdListCurrent->getState() == RHICommandListState::Recording)
        {
            // TODO: Add image barrier to convert to present src layout
            m_cmdListCurrent->insertBarrier(swapchain->getCurrentImage(),
                                            RHIFormat::B8R8G8A8Unorm,
                                            0,
                                            1,
                                            1,
                                            RHIImageLayout::PresentSource);
            m_cmdListCurrent->submit(swapchain->getImageAcquireSemaphore());
            swapchain->present(m_cmdListCurrent);
        }
    }

    void Renderer::setViewport(float const width, float const height)
    {
        WS_ASSERT((width != 0.0f) && (height != 0.0f));

        if ((viewport.width != width) || (viewport.height != height))
        {
            viewport.width  = width;
            viewport.height = height;
        }
    }

    RHIViewport const& Renderer::getViewport()
    {
        return viewport;
    }

} // namespace worse