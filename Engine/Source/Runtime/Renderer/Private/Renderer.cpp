#include "Math/Math.hpp"
#include "Math/Vector.hpp"
#include "Profiling/Stopwatch.hpp"
#include "RHIVertex.hpp"
#include "Window.hpp"
#include "Renderer.hpp"
#include "RHIQueue.hpp"
#include "RHIDevice.hpp"
#include "RHIViewport.hpp"
#include "RHISwapchain.hpp"
#include "RHICommandList.hpp"
#include "Descriptor/RHIBuffer.hpp"
#include "Descriptor/RHITexture.hpp"
#include "Descriptor/RHIDescriptor.hpp"
#include "Pipeline/RHIPipelineState.hpp"
#include "RendererBuffer.hpp"

#include <memory>

namespace worse
{
    struct FrameConstantData
    {
        float deltaTime;
        float time;
        Vector2 padding0; // align to 16 bytes

        Vector3 cameraPosition;
        float cameraNear;
        Vector3 cameraForward;
        float cameraFar;
        Vector4 padding1; // align to 16 bytes

        Matrix4 view;
        Matrix4 projection;
        Matrix4 viewProjection;
    };

    namespace
    {
        Vector2 resolutionRender = Vector2{0, 0};
        Vector2 resolutionOutput = Vector2{0, 0};
        RHIViewport viewport     = RHIViewport(0, 0, 0, 0);

        std::shared_ptr<RHISwapchain> swapchain = nullptr;
        RHICommandList* m_cmdList               = nullptr;

        FrameConstantData frameConstantData            = {};
        std::shared_ptr<RHIBuffer> frameConstantBuffer = nullptr;
        PushConstantData pushConstantData              = {};

        std::shared_ptr<RHIBuffer> testVbo = nullptr;
        std::shared_ptr<RHIBuffer> testIbo = nullptr;

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
                                static_cast<float>(Window::getWidth())};

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

            frameConstantData.time = 0.0;
            frameConstantBuffer =
                std::make_shared<RHIBuffer>(RHIBufferType::Constant,
                                            sizeof(FrameConstantData),
                                            1,
                                            &frameConstantData,
                                            true,
                                            "frameConstantBuffer");

            // clang-format off
            static const std::array<RHIVertexPosUvNrmTan, 4> vertices = {
                RHIVertexPosUvNrmTan{{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, { 1.0f, 0.0f, 0.0f}},
                RHIVertexPosUvNrmTan{{ 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
                RHIVertexPosUvNrmTan{{ 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, { 1.0f, 0.0f, 0.0f}},
                RHIVertexPosUvNrmTan{{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}}
            };
            static const std::array<std::uint32_t, 6> indices = {
                0, 1, 2,
                2, 3, 0
            };
            // clang-format on

            testVbo = std::make_shared<RHIBuffer>(RHIBufferType::Vertex,
                                                  sizeof(RHIVertexPosUvNrmTan),
                                                  vertices.size(),
                                                  vertices.data(),
                                                  true,
                                                  "testVbo");
            testIbo = std::make_shared<RHIBuffer>(RHIBufferType::Index,
                                                  sizeof(std::uint32_t),
                                                  indices.size(),
                                                  indices.data(),
                                                  true,
                                                  "testIbo");
        }

        RHIDevice::setResourceProvider(&resourceProvider);
    }

    void Renderer::shutdown()
    {
        RHIDevice::queueWaitAll();

        {
            frameConstantBuffer.reset();
            testVbo.reset();
            testIbo.reset();

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

        // update frame constant data
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
            RHIDevice::resetDescriptorAllocator();
            RHIDevice::writeGlobalDescriptorSet();
            RHIDevice::resetSpecificDescriptorSets();
        }

        RHITexture* frameRender =
            Renderer::getRenderTarget(RendererTarget::Render);
        RHITexture* frameOutput =
            Renderer::getRenderTarget(RendererTarget::Output);
        {
            // clang-format off
            static auto testPso = RHIPipelineStateBuilder()
                .setName("testPso")
                .setType(RHIPipelineType::Graphics)
                .setPrimitiveTopology(RHIPrimitiveTopology::Trianglelist)
                .setRasterizerState(Renderer::getRasterizerState(RendererRasterizerState::Solid))
                .setDepthStencilState(Renderer::getDepthStencilState(RendererDepthStencilState::Off))
                .setBlendState(Renderer::getBlendState(RendererBlendState::Off))
                .addShader(Renderer::getShader(RendererShader::PBRV))
                .addShader(Renderer::getShader(RendererShader::PBRP))
                .setRenderTargetColorTexture(0, frameRender)
                .setScissor({0, 0, 800, 600})
                .setViewport(Renderer::getViewport())
                .setClearColor(Color::Black())
                .build();

            std::array bindlessTextureupdates = {
                RHIDescriptorWrite{0, 0,
                {Renderer::getTexture(RendererTexture::TestA)}},
                RHIDescriptorWrite{0, 1,
                {Renderer::getTexture(RendererTexture::TestB)}},
            };
            RHIDevice::updateBindlessTextures(bindlessTextureupdates);
            // clang-format on

            m_cmdList->setPipelineState(testPso);
            m_cmdList->setBufferVertex(testVbo.get());
            m_cmdList->setBufferIndex(testIbo.get());
            m_cmdList->drawIndexed(testIbo->getElementCount(), 0, 0, 0, 1);

            m_cmdList->renderPassEnd();

            // ====================================
            // post pass
            // ====================================

            {
                static auto computePso =
                    RHIPipelineStateBuilder()
                        .setName("computePso")
                        .setType(RHIPipelineType::Compute)
                        .addShader(Renderer::getShader(RendererShader::TestC))
                        .build();

                m_cmdList->insertBarrier(frameRender->getImage(),
                                         frameRender->getFormat(),
                                         RHIImageLayout::ShaderRead);

                m_cmdList->insertBarrier(frameOutput->getImage(),
                                         frameOutput->getFormat(),
                                         RHIImageLayout::General);

                m_cmdList->setPipelineState(computePso);
                std::array updates = {
                    RHIDescriptorWrite{.reg      = 0, // t0
                                       .resource = {frameRender},
                                       .type     = RHIDescriptorType::Texture},
                    RHIDescriptorWrite{.reg      = 0, // u0
                                       .resource = {frameOutput},
                                       .type =
                                           RHIDescriptorType::TextureStorage},
                };
                m_cmdList->updateSpecificSet(updates);
                uint32_t groupCountX = (resolutionOutput.x + 7) / 8;
                uint32_t groupCountY = (resolutionOutput.y + 7) / 8;
                pushConstantData.setF30(Vector3{0.2, 0.3, 0.4});
                m_cmdList->pushConstants(
                    std::span<std::byte, RHIConfig::MAX_PUSH_CONSTANT_SIZE>(
                        reinterpret_cast<std::byte*>(&pushConstantData),
                        sizeof(PushConstantData)));
                m_cmdList->dispatch(groupCountX, groupCountY, 1);
            }

            blitToBackBuffer(m_cmdList);

            m_cmdList->clearPipelineState();
        }

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

    Vector2 Renderer::getResolutionRender()
    {
        return resolutionRender;
    }

    Vector2 Renderer::getResolutionOutput()
    {
        return resolutionOutput;
    }

} // namespace worse