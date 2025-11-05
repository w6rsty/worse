#include "glTF/glTF.hpp"
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

namespace Worse
{

    namespace
    {
        ULong frameCount = 0;

        Vector2 resolutionRender = Vector2::ZERO;
        Vector2 resolutionOutput = Vector2::ZERO;
        RHIViewport viewport     = RHIViewport(0, 0, 0, 0);

        std::shared_ptr<RHISwapchain> swapchain = nullptr;
        RHICommandList* m_currentCmdList        = nullptr;

        std::shared_ptr<RHIBuffer> frameConstantBuffer = nullptr;

        class RendererResourceProvider : public RHIResourceProvider
        {
        public:
            RendererResourceProvider()           = default;
            ~RendererResourceProvider() override = default;

            std::pair<RHIShader*, RHIShader*> getPlaceholderShader() const override
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
                samplers[RHISamplerType::CompareDepth]        = Renderer::getSampler(RHISamplerType::CompareDepth);
                samplers[RHISamplerType::PointClampBorder]    = Renderer::getSampler(RHISamplerType::PointClampBorder);
                samplers[RHISamplerType::PointClampEdge]      = Renderer::getSampler(RHISamplerType::PointClampEdge);
                samplers[RHISamplerType::PointWrap]           = Renderer::getSampler(RHISamplerType::PointWrap);
                samplers[RHISamplerType::BilinearClampEdge]   = Renderer::getSampler(RHISamplerType::BilinearClampEdge);
                samplers[RHISamplerType::BilinearClampBorder] = Renderer::getSampler(RHISamplerType::BilinearClampBorder);
                samplers[RHISamplerType::BilinearWrap]        = Renderer::getSampler(RHISamplerType::BilinearWrap);
                samplers[RHISamplerType::TrilinearClamp]      = Renderer::getSampler(RHISamplerType::TrilinearClamp);
                samplers[RHISamplerType::AnisotropicClamp]    = Renderer::getSampler(RHISamplerType::AnisotropicClamp);
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
            resolutionRender = Vector2{1200, 720};
            // output resolution
            resolutionOutput = Vector2{static_cast<Float>(Window::getWidth()), static_cast<Float>(Window::getHeight())};

            Renderer::setViewport(resolutionRender.x, resolutionRender.y);
        }

        // swapchain
        {
            swapchain = std::make_shared<RHISwapchain>(
                Window::getHandleSDL(),
                Window::getWidth(),
                Window::getHeight(),
                RHIConfig::enableVSync ? RHIPresentMode::FIFO : RHIPresentMode::Immediate,
                "default_swapchain");
        }

        // resources
        {
            FrameConstantData frameConstantData = {};
            frameConstantBuffer                 = std::make_shared<RHIBuffer>(
                RHIBufferUsage::FlagBits::Uniform,
                sizeof(FrameConstantData),
                1,
                &frameConstantData,
                true,
                "frame_constant_buffer");

            Renderer::createRasterizerStates();
            Renderer::createDepthStencilStates();
            Renderer::createBlendStates();
            Renderer::createRendererTarget();
            Renderer::createShaders();
            Renderer::createTextures();
            Renderer::createSamplers();
            Renderer::createStandardMeshes();
        }

        RHIDevice::setResourceProvider(&resourceProvider);

        commands.emplaceResource<GlobalContext>();
        commands.emplaceResource<DrawcallStorage>();
        commands.emplaceResourceArray<StandardMaterial>();
        commands.emplaceResourceArray<TextureWrite>();
        Worse::AssetServer& assetServer = commands.emplaceResource<AssetServer>();
        commands.emplaceResource<glTFManager>(assetServer);
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
            commands.removeResourceArray<TextureWrite>();
            commands.removeResource<AssetServer>();
            commands.removeResource<glTFManager>();
        }

        RHIDevice::destroy();
    }

    void Renderer::tick(
        ecs::Resource<DrawcallStorage> drawcalls,
        ecs::Resource<Camera> camera,
        ecs::Resource<GlobalContext> globalContext,
        ecs::ResourceArray<TextureWrite> textureWrites,
        ecs::Resource<AssetServer> assetServer)
    {
        // signal image acquire semaphore(swapchain)
        swapchain->acquireNextImage();

        RHIQueue* graphicsQueue = RHIDevice::getQueue(RHIQueueType::Graphics);
        m_currentCmdList        = graphicsQueue->nextCommandList();
        m_currentCmdList->begin();

        updateBuffers(m_currentCmdList, camera, globalContext, textureWrites);

        // render passes
        produceFrame(m_currentCmdList, globalContext, drawcalls, assetServer);

        // 将渲染结果拷贝到交换链图像
        blitToBackBuffer(m_currentCmdList);

        // [Sumbit] wait image acquire semaphore(swapchain)
        //          signal rendering semaphore(CommandList)
        // [Present] wait rendering semaphore(CommandList)
        Renderer::submitAndPresent();

        ++frameCount;
    } // namespace worse

    RHISwapchain* Renderer::getSwapchain()
    {
        return swapchain.get();
    }

    void Renderer::blitToBackBuffer(RHICommandList* cmdList)
    {
        cmdList->blit(getRenderTarget(RendererTarget::ScreenHDR), swapchain.get());
    }

    void Renderer::submitAndPresent()
    {
        if (m_currentCmdList->getState() == RHICommandListState::Recording)
        {
            m_currentCmdList->insertBarrier(swapchain->getCurrentRt(), RHIFormat::B8R8G8A8Unorm, RHIImageLayout::PresentSource, RHIPipelineStage::FlagBits::AllCommands, RHIAccessUsage::FlagBits::MemoryWrite, RHIPipelineStage::FlagBits::BottomOfPipe, RHIAccessUsage::FlagBits::MemoryRead);
            m_currentCmdList->submit(swapchain->getImageAcquireSemaphore());
            swapchain->present(m_currentCmdList);
        }
    }

    void Renderer::writeBindlessTextures(ecs::ResourceArray<TextureWrite> textureWrites)
    {
        std::vector<RHIDescriptorWrite> updates;
        updates.reserve(static_cast<Size>(RendererTexture::Max) + textureWrites->data().size());

        // builtin textures (0-5)
        // clang-format off
        updates.emplace_back(0, 0, RHIDescriptorResource{Renderer::getTexture(RendererTexture::Placeholder)},              RHIDescriptorType::Texture);
        updates.emplace_back(0, 1, RHIDescriptorResource{Renderer::getTexture(RendererTexture::DefaultAlbedo)},            RHIDescriptorType::Texture);
        updates.emplace_back(0, 2, RHIDescriptorResource{Renderer::getTexture(RendererTexture::DefaultNormal)},            RHIDescriptorType::Texture);
        updates.emplace_back(0, 3, RHIDescriptorResource{Renderer::getTexture(RendererTexture::DefaultMetallicRoughness)}, RHIDescriptorType::Texture);
        updates.emplace_back(0, 4, RHIDescriptorResource{Renderer::getTexture(RendererTexture::DefaultAmbientOcclusion)},  RHIDescriptorType::Texture);
        updates.emplace_back(0, 5, RHIDescriptorResource{Renderer::getTexture(RendererTexture::DefaultEmissive)},          RHIDescriptorType::Texture);
        
        // dynamic textures
        for (auto const& textureWrite : textureWrites->data())
        {   
            updates.emplace_back(0, textureWrite.index, RHIDescriptorResource{textureWrite.texture}, RHIDescriptorType::Texture);
        }
        // clang-format on

        RHIDevice::updateBindlessTextures(updates);
    }

    void Renderer::updateBuffers(
        RHICommandList* cmdList,
        ecs::Resource<Camera> camera,
        ecs::Resource<GlobalContext> globalContext,
        ecs::ResourceArray<TextureWrite> textureWrites)
    {
        // update frame constant data

        FrameConstantData frameConstantData     = {};
        frameConstantData.cameraPosition        = camera->getPosition();
        frameConstantData.cameraForward         = camera->getForward();
        frameConstantData.deltaTime             = globalContext->deltaTime;
        frameConstantData.time                  = globalContext->time;
        frameConstantData.projection            = camera->getProjectionMatrix();
        frameConstantData.view                  = camera->getViewMatrix();
        frameConstantData.viewProjection        = frameConstantData.projection * frameConstantData.view;
        frameConstantData.viewProjectionInverse = Inverse(frameConstantData.viewProjection);

        auto mat = frameConstantData.viewProjection * frameConstantData.viewProjectionInverse;

        m_currentCmdList->updateBuffer(frameConstantBuffer.get(), 0, sizeof(FrameConstantData), &frameConstantData);

        // prepare descriptor

        // Reset descriptor pool
        RHIDevice::resetDescriptorAllocator();

        RHIDevice::writeGlobalDescriptorSet();
        Renderer::writeBindlessTextures(textureWrites);

        RHIDevice::resetSpecificDescriptorSets();
    }

    void Renderer::setViewport(Float const width, Float const height)
    {
        WORSE_ASSERT((width != 0.0f) && (height != 0.0f));

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

    Vector2 Renderer::getResolutionRender()
    {
        return resolutionRender;
    }

    Vector2 Renderer::getResolutionOutput()
    {
        return resolutionOutput;
    }

} // namespace Worse