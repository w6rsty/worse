#include "Math/Math.hpp"
#include "Profiling/Stopwatch.hpp"
#include "Window.hpp"
#include "Renderer.hpp"
#include "RHIQueue.hpp"
#include "RHIDevice.hpp"
#include "RHIViewport.hpp"
#include "RHISwapchain.hpp"
#include "RHICommandList.hpp"
#include "Descriptor/RHIBuffer.hpp"
#include "Descriptor/RHITexture.hpp"
#include "RendererBuffer.hpp"

#include <memory>

namespace worse
{

    namespace
    {
        math::Vector2 resolutionRender = math::Vector2{0, 0};
        math::Vector2 resolutionOutput = math::Vector2{0, 0};
        RHIViewport viewport           = RHIViewport(0, 0, 0, 0);

        std::shared_ptr<RHISwapchain> swapchain = nullptr;
        RHICommandList* m_cmdList               = nullptr;

        FrameConstantData frameConstantData            = {};
        std::shared_ptr<RHIBuffer> frameConstantBuffer = nullptr;

        class RendererResourceProvider : public RHIResourceProvider
        {
        public:
            RendererResourceProvider()           = default;
            ~RendererResourceProvider() override = default;

            std::pair<RHIShader*, RHIShader*>
            getPlaceholderShader() const override
            {
                return {Renderer::getShader(RendererShader::PlaceholderV),
                        Renderer::getShader(RendererShader::PlaceholderP)};
            }

            RHITexture* getPlaceholderTexture() const override
            {
                return Renderer::getTexture(RendererTexture::Placeholder);
            }

            RHIBuffer* getFrameConstantBuffer() const override
            {
                return frameConstantBuffer.get();
            }

            EnumArray<RHISamplerType, RHISampler*> getSamplers() const override
            {
                EnumArray<RHISamplerType, RHISampler*> samplers;
                // clang-format off
                samplers[RHISamplerType::CompareDepth]        = Renderer::getSampler(RHISamplerType::CompareDepth);
                samplers[RHISamplerType::PointClampBorder]    = Renderer::getSampler(RHISamplerType::PointClampBorder);
                samplers[RHISamplerType::PointClampEdge]      = Renderer::getSampler(RHISamplerType::PointClampEdge);
                samplers[RHISamplerType::Wrap]                = Renderer::getSampler(RHISamplerType::Wrap);
                samplers[RHISamplerType::BilinearClampEdge]   = Renderer::getSampler(RHISamplerType::BilinearClampEdge);
                samplers[RHISamplerType::BilinearClampBorder] = Renderer::getSampler(RHISamplerType::BilinearClampBorder);
                samplers[RHISamplerType::BilinearWrap]        = Renderer::getSampler(RHISamplerType::BilinearWrap);
                samplers[RHISamplerType::TrilinearClamp]      = Renderer::getSampler(RHISamplerType::TrilinearClamp);
                samplers[RHISamplerType::AnisotropicClamp]    = Renderer::getSampler(RHISamplerType::AnisotropicClamp);
                // clang-format on
                return samplers;
            }
        } resourceProvider;

    } // namespace

    void Renderer::initialize()
    {
        RHIDevice::initialize();

        // resolution
        {
            // render resolution
            resolutionRender = {800, 600};
            // output resolution
            resolutionOutput = {static_cast<float>(Window::getWidth()),
                                static_cast<float>(Window::getHeight())};

            Renderer::setViewport(resolutionRender.x, resolutionRender.y);
        }

        // swapchain
        {
            swapchain = std::make_shared<RHISwapchain>(
                Window::getHandleSDL(),
                Window::getWidth(),
                Window::getHeight(),
                RHIConfig::enableVSync ? RHIPresentMode::FIFO
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
            Renderer::createTextures();
            Renderer::createSamplers();
            Renderer::createStandardMeshes();

            frameConstantBuffer =
                std::make_shared<RHIBuffer>(RHIBufferUsageFlagBits::Uniform,
                                            sizeof(FrameConstantData),
                                            1,
                                            &frameConstantData,
                                            true,
                                            "frameConstantBuffer");
        }

        RHIDevice::setResourceProvider(&resourceProvider);
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

        updateBuffers(m_cmdList);

        // render passes
        produceFrame(m_cmdList);

        blitToBackBuffer(m_cmdList);

        // [Sumbit] wait image acquire semaphore(swapchain)
        //          signal rendering semaphore(CommandList)
        // [Present] wait rendering semaphore(CommandList)
        Renderer::submitAndPresent();

        ++s_frameCount;
    } // namespace worse

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

    void Renderer::updateBuffers(RHICommandList* cmdList)
    {
        // update frame constant data

        static profiling::Stopwatch frameTimer;
        frameConstantData.deltaTime = frameTimer.elapsedSec();
        frameConstantData.time += frameConstantData.deltaTime;
        frameConstantData.projection =
            math::projectionPerspective(math::toRadians(70.0f),
                                        viewport.width / viewport.height,
                                        0.1f,
                                        100.0f);


        frameConstantData.view = math::lookAt(frameConstantData.cameraPosition,
                                              math::Vector3{0, 0, 0},
                                              math::Vector3{0, 1, 0});
        frameConstantData.viewProjection =
            frameConstantData.projection * frameConstantData.view;
        frameTimer.reset();

        m_cmdList->updateBuffer(frameConstantBuffer.get(),
                                0,
                                sizeof(FrameConstantData),
                                &frameConstantData);

        // prepare descriptor

        RHIDevice::resetDescriptorAllocator();
        RHIDevice::writeGlobalDescriptorSet();
        RHIDevice::resetSpecificDescriptorSets();
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

    void Renderer::setCameraPosition(math::Vector3 const& position)
    {
        frameConstantData.cameraPosition = position;
    }

} // namespace worse