#include "Math/Vector2.hpp"
#include "Profiling/Stopwatch.hpp"
#include "Window.hpp"
#include "Renderer.hpp"
#include "RHIQueue.hpp"
#include "RHIDevice.hpp"
#include "RHIViewport.hpp"
#include "RHISwapchain.hpp"
#include "RHICommandList.hpp"
#include "Pipeline/RHIPipelineState.hpp"
#include "Descriptor/RHIBuffer.hpp"

#include <memory>

namespace worse
{
    struct alignas(16) FrameConstantData
    {
        float deltaTime;
        float time;
    };

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
        RHICommandList* m_cmdList               = nullptr;

        FrameConstantData frameConstantData            = {};
        std::shared_ptr<RHIBuffer> frameConstantBuffer = nullptr;

        RHIPipelineState testPso;
    } // namespace

    void Renderer::initialize()
    {
        RHIDevice::initialize();

        // resolution
        {
            std::uint32_t width  = Window::getWidth();
            std::uint32_t height = Window::getHeight();

            // render resolution
            resolutionRender = {800, 600};
            // output resolution
            resolutionOutput = {static_cast<float>(width),
                                static_cast<float>(height)};

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
                "swapchain");
        }

        // resources
        {
            Renderer::createRasterizerStates();
            Renderer::createDepthStencilStates();
            Renderer::createBlendStates();
            Renderer::createRendererTarget();
            Renderer::createShaders();

            frameConstantData.time = 0.0;
            frameConstantBuffer =
                std::make_shared<RHIBuffer>(RHIBufferType::Constant,
                                            sizeof(FrameConstantData),
                                            1,
                                            &frameConstantData,
                                            true,
                                            "frameConstantBuffer");
        }

        // clang-format off
        RHITexture* frameOutput = Renderer::getRenderTarget(RendererTarget::Output);

        testPso = RHIPipelineStateBuilder()
            .setName("testPSO")
            .setType(RHIPipelineType::Graphics)
            .setPrimitiveTopology(RHIPrimitiveTopology::Trianglelist)
            .setRasterizerState(Renderer::getRasterizerState(RendererRasterizerState::Solid))
            .setDepthStencilState(Renderer::getDepthStencilState(RendererDepthStencilState::Off))
            .setBlendState(Renderer::getBlendState(RendererBlendState::Off))
            .addShader(Renderer::getShader(RendererShader::QuadV))
            .addShader(Renderer::getShader(RendererShader::QuadP))
            .setRenderTargetColorTexture(0, frameOutput)
            .setScissor({0, 0, 800, 600})
            .setViewport(Renderer::getViewport())
            .setClearColor(Color::Black())
            .build();
        // clang-format on
    }

    void Renderer::shutdown()
    {
        RHIDevice::queueWaitAll();

        {
            frameConstantBuffer.reset();
            destroyResources();
            swapchain.reset();
        }

        RHIDevice::destroy();

        WS_LOG_DEBUG("Renderer", "Finished {} frame", s_frameCount);
    }

    void Renderer::tick()
    {
        // signal image acquire semaphore(swapchain)
        swapchain->acquireNextImage();

        RHIQueue* graphicsQueue = RHIDevice::getQueue(RHIQueueType::Graphics);
        m_cmdList               = graphicsQueue->nextCommandList();
        m_cmdList->begin();

        if (s_frameCount != 0)
        {
            RHIDevice::deletionQueueFlush();
        }

        // update buffers
        {
            static profiling::Stopwatch frameTimer;
            frameConstantData.deltaTime = frameTimer.elapsedSec();
            frameConstantData.time += frameConstantData.deltaTime;
            frameTimer.reset();

            m_cmdList->updateBuffer(frameConstantBuffer.get(),
                                    0,
                                    sizeof(FrameConstantData),
                                    &frameConstantData);
        }

        {
            RHITexture* frameOutput =
                Renderer::getRenderTarget(RendererTarget::Output);

            m_cmdList->setPipelineState(testPso, frameConstantBuffer.get());
            m_cmdList->draw(6);
            m_cmdList->renderPassEnd();
            m_cmdList->clearPipelineState();
        }

        blitToBackBuffer(m_cmdList);

        // [Sumbit] wait image acquire semaphore(swapchain)
        //          signal rendering semaphore(CommandList)
        // [Present] wait rendering semaphore(CommandList)
        Renderer::submitAndPresent();

        ++s_frameCount;
    }

    RHISwapchain* Renderer::getSwapchain()
    {
        return swapchain.get();
    }

    void Renderer::blitToBackBuffer(RHICommandList* cmdList)
    {
        cmdList->blit(getRenderTarget(RendererTarget::Output), swapchain.get());
    }

    void Renderer::submitAndPresent()
    {
        if (m_cmdList->getState() == RHICommandListState::Recording)
        {
            m_cmdList->insertBarrier(swapchain->getCurrentRt(),
                                     RHIFormat::B8R8G8A8Unorm,
                                     RHIImageLayout::PresentSource);
            m_cmdList->submit(swapchain->getImageAcquireSemaphore());
            swapchain->present(m_cmdList);
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

    void Renderer::setFrameConstantData(RHICommandList* cmdList)
    {
        cmdList->setContantBuffer(frameConstantBuffer.get(), 0);
    }

    math::Vector2 Renderer::getResolutionRender()
    {
        return resolutionRender;
    }

    math::Vector2 Renderer::getResolutionOutput()
    {
        return resolutionOutput;
    }

} // namespace worse