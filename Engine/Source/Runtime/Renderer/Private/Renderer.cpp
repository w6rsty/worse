#include "DXCompiler.hpp" // Do not move
#include "RHIDefinitions.hpp"
#include "Renderer.hpp"
#include "Window.hpp"
#include "RHIDevice.hpp"
#include "RHIViewport.hpp"
#include "RHISwapchain.hpp"
#include "RHIQueue.hpp"
#include "Math/Vector2.hpp"
#include "Pipeline/RHIPipelineState.hpp"

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
        RHICommandList* m_cmdList               = nullptr;
    } // namespace

    void Renderer::initialize()
    {
        RHIDevice::initialize();
        DXCompiler::initialize();

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
        }
    }

    void Renderer::shutdown()
    {
        RHIDevice::queueWaitAll();

        {
            destroyResources();
            swapchain.reset();
        }

        DXCompiler::shutdown();
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

        {
            // clang-format off
            RHITexture* frameRender = Renderer::getRenderTarget(RendererTarget::Render);
            RHITexture* frameOutput = Renderer::getRenderTarget(RendererTarget::Output);

            RHIPipelineState testPso;
            testPso.name                                                     = "testPSO";
            testPso.type                                                     = RHIPipelineType::Graphics;
            testPso.shaders[static_cast<std::size_t>(RHIShaderType::Vertex)] = Renderer::getShader(RendererShader::QuadV);
            testPso.shaders[static_cast<std::size_t>(RHIShaderType::Pixel)]  = Renderer::getShader(RendererShader::QuadP);
            testPso.rasterizerState   = Renderer::getRasterizerState(RendererRasterizerState::Solid);
            testPso.depthStencilState = Renderer::getDepthStencilState(RendererDepthStencilState::Off);
            testPso.blendState        = Renderer::getBlendState(RendererBlendState::Off);
            testPso.scissor           = {0, 0, 800, 600};
            testPso.viewport          = Renderer::getViewport();
            testPso.clearColor        = Color::Black();
            testPso.renderTargetColorTextures[0] = frameRender;
            // clang-format on
            testPso.finalize();

            m_cmdList->setPipelineState(testPso);
            m_cmdList->draw(3);
            m_cmdList->renderPassEnd();
            m_cmdList->clearPipelineState();

            m_cmdList->blit(frameRender, frameOutput);
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

    math::Vector2 Renderer::getResolutionRender()
    {
        return resolutionRender;
    }

    math::Vector2 Renderer::getResolutionOutput()
    {
        return resolutionOutput;
    }

} // namespace worse