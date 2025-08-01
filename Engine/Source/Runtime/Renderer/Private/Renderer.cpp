#include "Log.hpp"
#include "Window.hpp"
#include "RHIQueue.hpp"
#include "RHIDevice.hpp"
#include "RHIViewport.hpp"
#include "RHISwapchain.hpp"
#include "RHICommandList.hpp"
#include "RHIBuffer.hpp"
#include "RHITexture.hpp"
#include "Renderer.hpp"
#include "RendererBuffer.hpp"
#include "AssetServer.hpp"

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

    void Renderer::initialize(ecs::Commands commands)
    {
        RHIDevice::initialize();

        // resolution
        {
            // render resolution
            resolutionRender = {1200, 720};
            // output resolution
            resolutionOutput = {static_cast<f32>(Window::getWidth()),
                                static_cast<f32>(Window::getHeight())};

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
            frameConstantBuffer = std::make_shared<RHIBuffer>(RHIBufferUsageFlagBits::Uniform,
                                                              sizeof(FrameConstantData),
                                                              1,
                                                              &frameConstantData,
                                                              true,
                                                              "frameConstantBuffer");

            Renderer::createRasterizerStates();
            Renderer::createDepthStencilStates();
            Renderer::createBlendStates();
            Renderer::createRendererTarget();
            Renderer::createShaders();
            Renderer::createTextures();
            Renderer::createSamplers();
            Renderer::createStandardMeshes();
            Renderer::createPipelineStates();
        }

        RHIDevice::setResourceProvider(&resourceProvider);

        commands.emplaceResource<GlobalContext>();
        commands.emplaceResource<DrawcallStorage>();
        commands.emplaceResourceArray<StandardMaterial>();
        commands.emplaceResourceArray<StandardMaterialGPU>();
        commands.emplaceResourceArray<TextureWrite>();
        commands.emplaceResource<AssetServer>();
    }

    void Renderer::shutdown(ecs::Commands commands)
    {
        RHIDevice::queueWaitAll();
        {
            frameConstantBuffer.reset();

            destroyResources();
            swapchain.reset();

            commands.removeResource<GlobalContext>();
            commands.removeResource<DrawcallStorage>();
            commands.removeResourceArray<StandardMaterial>();
            commands.removeResourceArray<StandardMaterialGPU>();
            commands.removeResourceArray<TextureWrite>();

            commands.removeResource<AssetServer>();
        }

        RHIDevice::destroy();

        WS_LOG_DEBUG("Renderer", "Finished {} frame", s_frameCount);
    }

    void Renderer::tick(ecs::Resource<DrawcallStorage> drawcalls,
                        ecs::Resource<Camera> camera,
                        ecs::Resource<GlobalContext> globalContext,
                        ecs::ResourceArray<TextureWrite> textureWrites)
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

        updateBuffers(m_cmdList, camera, globalContext);

        // render passes
        produceFrame(m_cmdList, globalContext, drawcalls, textureWrites);

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
            m_cmdList->insertBarrier(swapchain->getCurrentRt(), RHIFormat::B8R8G8A8Unorm, RHIImageLayout::PresentSource);
            m_cmdList->submit(swapchain->getImageAcquireSemaphore());
            swapchain->present(m_cmdList);
        }
    }

    void Renderer::writeBindlessTextures(ecs::ResourceArray<TextureWrite> textureWrites)
    {
        std::vector<RHIDescriptorWrite> updates;
        updates.reserve(static_cast<usize>(RendererTexture::Max) + textureWrites->data().size());

        // builtin textures (0-6)
        // clang-format off
        updates.emplace_back(0, 0, RHIDescriptorResource{Renderer::getTexture(RendererTexture::Placeholder)},             RHIDescriptorType::Texture);
        updates.emplace_back(0, 1, RHIDescriptorResource{Renderer::getTexture(RendererTexture::DefaultAlbedo)},           RHIDescriptorType::Texture);
        updates.emplace_back(0, 2, RHIDescriptorResource{Renderer::getTexture(RendererTexture::DefaultNormal)},           RHIDescriptorType::Texture);
        updates.emplace_back(0, 3, RHIDescriptorResource{Renderer::getTexture(RendererTexture::DefaultMetallic)},         RHIDescriptorType::Texture);
        updates.emplace_back(0, 4, RHIDescriptorResource{Renderer::getTexture(RendererTexture::DefaultRoughness)},        RHIDescriptorType::Texture);
        updates.emplace_back(0, 5, RHIDescriptorResource{Renderer::getTexture(RendererTexture::DefaultAmbientOcclusion)}, RHIDescriptorType::Texture);
        updates.emplace_back(0, 6, RHIDescriptorResource{Renderer::getTexture(RendererTexture::DefaultEmissive)},         RHIDescriptorType::Texture);
        
        // dynamic textures
        for (auto const& textureWrite : textureWrites->data())
        {   
            updates.emplace_back(0, textureWrite.index, RHIDescriptorResource{textureWrite.texture}, RHIDescriptorType::Texture);
        }
        // clang-format on

        RHIDevice::updateBindlessTextures(updates);
    }

    void Renderer::updateBuffers(RHICommandList* cmdList, ecs::Resource<Camera> camera, ecs::Resource<GlobalContext> globalContext)
    {
        // update frame constant data

        frameConstantData.deltaTime  = globalContext->deltaTime;
        frameConstantData.time       = globalContext->time;
        frameConstantData.projection = camera->getProjectionMatrix();

        frameConstantData.view           = camera->getViewMatrix();
        frameConstantData.viewProjection = frameConstantData.projection * frameConstantData.view;

        m_cmdList->updateBuffer(frameConstantBuffer.get(), 0, sizeof(FrameConstantData), &frameConstantData);

        // prepare descriptor

        RHIDevice::resetDescriptorAllocator();
        RHIDevice::writeGlobalDescriptorSet();
        RHIDevice::resetSpecificDescriptorSets();
    }

    void Renderer::setViewport(f32 const width, f32 const height)
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

    RHIFormat Renderer::getSwapchainFormat()
    {
        return swapchain->getFormat();
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

    void Renderer::setCameraForward(math::Vector3 const& forward)
    {
        frameConstantData.cameraForward = forward;
    }

} // namespace worse