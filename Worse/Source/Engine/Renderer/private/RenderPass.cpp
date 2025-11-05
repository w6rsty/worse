#include "imgui.h"

#include "RHICommandList.hpp"
#include "RHITexture.hpp"
#include "RHIBuffer.hpp"
#include "Renderer.hpp"
#include "RendererBuffer.hpp"
namespace Worse
{

    namespace
    {
        PushConstantData pushConstantData = {};
    }

    void Renderer::setPushParameters(Float a, Float b)
    {
        pushConstantData.setPadding(a, b);
    }

    void Renderer::passDepthPrepass(RHICommandList* cmdList, ecs::Resource<DrawcallStorage> drawcalls)
    {
        RHITexture* depthTexture = Renderer::getRenderTarget(RendererTarget::DepthGBuffer);

        cmdList->setPipelineState(
            RHIPipelineStateBuilder()
                .setName("DepthPrepass")
                .setType(RHIPipelineType::Graphics)
                .setPrimitiveTopology(RHIPrimitiveTopology::TriangleList)
                .setRasterizerState(Renderer::getRasterizerState(RendererRasterizerState::DepthPrepass))
                .setDepthStencilState(Renderer::getDepthStencilState(RendererDepthStencilState::ReadWrite))
                .setBlendState(Renderer::getBlendState(RendererBlendState::Off))
                .addShader(Renderer::getShader(RendererShader::DepthPrepassV))
                .addShader(Renderer::getShader(RendererShader::DepthPrepassP))
                .setRenderTargetDepthTexture(depthTexture)
                .setScissor({0, 0, depthTexture->getWidth(), depthTexture->getHeight()})
                .setViewport(Renderer::getViewport())
                .setClearDepth(0.0f) // clear with far value
                .build());

        for (Drawcall const& drawcall : drawcalls->solid)
        {
            if (Mesh* mesh = drawcall.mesh)
            {
                cmdList->setBufferVertex(mesh->getVertexBuffer());
                cmdList->setBufferIndex(mesh->getIndexBuffer());

                pushConstantData.setTransform(drawcall.transform);
                cmdList->pushConstants(pushConstantData.asSpan());

                cmdList->drawIndexed(mesh->getIndexBuffer()->getElementCount(), 0, 0, 0, 1);
            }
        }

        for (RenderObject const& object : drawcalls->ctx.opaqueObjects)
        {
            cmdList->setBufferVertex(object.mesh->getVertexBuffer());
            cmdList->setBufferIndex(object.mesh->getIndexBuffer());

            pushConstantData.setTransform(object.transform);
            cmdList->pushConstants(pushConstantData.asSpan());

            cmdList->drawIndexed(object.indexCount, object.startIndex, 0, 0, 1);
        }

        cmdList->renderPassEnd();

        cmdList->insertBarrier(
            depthTexture->getImage(),
            depthTexture->getFormat(),
            RHIImageLayout::ShaderRead,
            RHIPipelineStage::FlagBits::AllGraphics,
            RHIAccessUsage::FlagBits::MemoryWrite,
            RHIPipelineStage::FlagBits::AllGraphics,
            RHIAccessUsage::FlagBits::MemoryRead);
    }

    void Renderer::passShadowMap(RHICommandList* cmdList, ecs::Resource<DrawcallStorage> drawcalls)
    {
        RHITexture* depthLight = Renderer::getRenderTarget(RendererTarget::DepthLight);

        cmdList->setPipelineState(
            RHIPipelineStateBuilder()
                .setName("DepthLight")
                .setType(RHIPipelineType::Graphics)
                .setPrimitiveTopology(RHIPrimitiveTopology::TriangleList)
                .setRasterizerState(Renderer::getRasterizerState(RendererRasterizerState::SolidCullBack))
                .setDepthStencilState(Renderer::getDepthStencilState(RendererDepthStencilState::ReadWrite))
                .setBlendState(Renderer::getBlendState(RendererBlendState::Off))
                .addShader(Renderer::getShader(RendererShader::DepthLightV))
                .addShader(Renderer::getShader(RendererShader::DepthLightP))
                .setRenderTargetDepthTexture(depthLight)
                .setScissor({0, 0, depthLight->getWidth(), depthLight->getHeight()})
                .setViewport(Renderer::getViewport())
                .setClearDepth(0.0f) // clear with far value
                .build());

        static Matrix4 lightSpaceMatrix =
            math::projectionOrtho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 100.0f) *
            math::lookAt(
                Vector3(-0.3f, -1.0f, -0.5f),
                Vector3(0.0f, 0.0f, 0.0f),
                Vector3(0, 1, 0));
        pushConstantData.setMatrix(lightSpaceMatrix);

        for (Drawcall const& drawcall : drawcalls->solid)
        {
            if (Mesh* mesh = drawcall.mesh)
            {
                cmdList->setBufferVertex(mesh->getVertexBuffer());
                cmdList->setBufferIndex(mesh->getIndexBuffer());

                pushConstantData.setTransform(drawcall.transform);
                cmdList->pushConstants(pushConstantData.asSpan());

                cmdList->drawIndexed(mesh->getIndexBuffer()->getElementCount(), 0, 0, 0, 1);
            }
        }

        for (RenderObject const& object : drawcalls->ctx.opaqueObjects)
        {
            cmdList->setBufferVertex(object.mesh->getVertexBuffer());
            cmdList->setBufferIndex(object.mesh->getIndexBuffer());

            pushConstantData.setTransform(object.transform);
            cmdList->pushConstants(pushConstantData.asSpan());

            cmdList->drawIndexed(object.indexCount, object.startIndex, 0, 0, 1);
        }

        cmdList->renderPassEnd();

        cmdList->insertBarrier(
            depthLight->getImage(),
            depthLight->getFormat(),
            RHIImageLayout::ShaderRead,
            RHIPipelineStage::FlagBits::FragmentShader,
            RHIAccessUsage::FlagBits::ShaderWrite,
            RHIPipelineStage::FlagBits::ComputeShader,
            RHIAccessUsage::FlagBits::ShaderSampledRead);
    }

    void Renderer::passGBuffer(RHICommandList* cmdList, ecs::Resource<DrawcallStorage> drawcalls, ecs::Resource<AssetServer> assetServer)
    {
        RHITexture* gbufferAlbedo   = Renderer::getRenderTarget(RendererTarget::GBufferAlbedo);
        RHITexture* gbufferNormal   = Renderer::getRenderTarget(RendererTarget::GBufferNormal);
        RHITexture* gbufferMaterial = Renderer::getRenderTarget(RendererTarget::GBufferMaterial);
        RHITexture* depthTexture    = Renderer::getRenderTarget(RendererTarget::DepthGBuffer);
        RHITexture* depthLight      = Renderer::getRenderTarget(RendererTarget::DepthLight);
        RHITexture* gbufferPosition = Renderer::getRenderTarget(RendererTarget::GBufferPosition);

        cmdList->setPipelineState(
            RHIPipelineStateBuilder()
                .setName("GBuffer")
                .setType(RHIPipelineType::Graphics)
                .setPrimitiveTopology(RHIPrimitiveTopology::TriangleList)
                .setRasterizerState(Renderer::getRasterizerState(RendererRasterizerState::SolidCullBack))
                .setDepthStencilState(Renderer::getDepthStencilState(RendererDepthStencilState::ReadGreaterEqual))
                .setBlendState(Renderer::getBlendState(RendererBlendState::Off))
                .addShader(Renderer::getShader(RendererShader::GBufferV))
                .addShader(Renderer::getShader(RendererShader::GBufferP))
                .setRenderTargetColorTexture(0, gbufferAlbedo)
                .setRenderTargetColorTexture(1, gbufferNormal)
                .setRenderTargetColorTexture(2, gbufferMaterial)
                .setRenderTargetColorTexture(3, gbufferPosition)
                .setRenderTargetDepthTexture(depthTexture)
                .setScissor({0, 0, gbufferAlbedo->getWidth(), gbufferAlbedo->getHeight()})
                .setViewport(Renderer::getViewport())
                .setClearColor(Color{0.02f, 0.02f, 0.02f, 1.0f})
                .setClearDepth(2.0f)
                .build());

        std::array updates = {
            RHIDescriptorWrite{.reg      = 0, // t0
                               .resource = {Renderer::getMaterialBuffer()},
                               .type     = RHIDescriptorType::StructuredBuffer},
        };
        cmdList->updateSpecificSet(updates);

        static Matrix4 lightSpaceMatrix =
            math::projectionOrtho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 100.0f) *
            math::lookAt(
                Vector3(-0.3f, -1.0f, -0.5f),
                Vector3(0.0f, 0.0f, 0.0f),
                Vector3(0, 1, 0));
        pushConstantData.setMatrix(lightSpaceMatrix);

        for (Drawcall const& drawcall : drawcalls->solid)
        {
            if (Mesh* mesh = drawcall.mesh)
            {
                cmdList->setBufferVertex(mesh->getVertexBuffer());
                cmdList->setBufferIndex(mesh->getIndexBuffer());

                pushConstantData.setTransform(drawcall.transform);
                pushConstantData.setMaterialId(drawcall.materialIndex);
                cmdList->pushConstants(pushConstantData.asSpan());

                cmdList->drawIndexed(mesh->getIndexBuffer()->getElementCount(), 0, 0, 0, 1);
            }
        }

        for (RenderObject const& object : drawcalls->ctx.opaqueObjects)
        {
            cmdList->setBufferVertex(object.mesh->getVertexBuffer());
            cmdList->setBufferIndex(object.mesh->getIndexBuffer());

            pushConstantData.setTransform(object.transform);
            pushConstantData.setMaterialId(assetServer->getMaterialIndex(object.material));
            cmdList->pushConstants(pushConstantData.asSpan());

            cmdList->drawIndexed(object.indexCount, object.startIndex, 0, 0, 1);
        }

        cmdList->setPipelineState(
            RHIPipelineStateBuilder()
                .setName("Point")
                .setType(RHIPipelineType::Graphics)
                .setPrimitiveTopology(RHIPrimitiveTopology::PointList)
                .setRasterizerState(Renderer::getRasterizerState(RendererRasterizerState::SolidCullBack))
                .setDepthStencilState(Renderer::getDepthStencilState(RendererDepthStencilState::ReadGreaterEqual))
                .setBlendState(Renderer::getBlendState(RendererBlendState::Off))
                .addShader(Renderer::getShader(RendererShader::PointV))
                .addShader(Renderer::getShader(RendererShader::PointP))
                .setRenderTargetColorTexture(0, gbufferAlbedo)
                .setRenderTargetDepthTexture(depthTexture)
                .setScissor({0, 0, gbufferAlbedo->getWidth(), gbufferAlbedo->getHeight()})
                .setViewport(Renderer::getViewport())
                .setClearDepth(2.0f)
                .build());

        for (Drawcall const& drawcall : drawcalls->point)
        {
            if (Mesh* mesh = drawcall.mesh)
            {
                cmdList->setBufferVertex(mesh->getVertexBuffer());

                pushConstantData.setTransform(drawcall.transform);
                pushConstantData.setMaterialId(drawcall.materialIndex);

                pushConstantData.setPadding(2.0f, 0.0f); // Point size
                cmdList->pushConstants(pushConstantData.asSpan());

                cmdList->draw(mesh->getVertexBuffer()->getElementCount());
            }
        }

        cmdList->renderPassEnd();

        cmdList->insertBarrier(
            gbufferAlbedo->getImage(),
            gbufferAlbedo->getFormat(),
            RHIImageLayout::ShaderRead,
            RHIPipelineStage::FlagBits::FragmentShader,
            RHIAccessUsage::FlagBits::ShaderWrite,
            RHIPipelineStage::FlagBits::ComputeShader,
            RHIAccessUsage::FlagBits::ShaderSampledRead);
        cmdList->insertBarrier(
            gbufferNormal->getImage(),
            gbufferNormal->getFormat(),
            RHIImageLayout::ShaderRead,
            RHIPipelineStage::FlagBits::FragmentShader,
            RHIAccessUsage::FlagBits::ShaderWrite,
            RHIPipelineStage::FlagBits::ComputeShader,
            RHIAccessUsage::FlagBits::ShaderSampledRead);
        cmdList->insertBarrier(
            gbufferMaterial->getImage(),
            gbufferMaterial->getFormat(),
            RHIImageLayout::ShaderRead,
            RHIPipelineStage::FlagBits::FragmentShader,
            RHIAccessUsage::FlagBits::ShaderWrite,
            RHIPipelineStage::FlagBits::ComputeShader,
            RHIAccessUsage::FlagBits::ShaderSampledRead);
    }

    void Renderer::passLight(RHICommandList* cmdList)
    {
        RHITexture* gbufferAlbedo   = Renderer::getRenderTarget(RendererTarget::GBufferAlbedo);
        RHITexture* gbufferNormal   = Renderer::getRenderTarget(RendererTarget::GBufferNormal);
        RHITexture* gbufferMaterial = Renderer::getRenderTarget(RendererTarget::GBufferMaterial);
        RHITexture* depthGBuffer    = Renderer::getRenderTarget(RendererTarget::DepthGBuffer);
        RHITexture* depthLight      = Renderer::getRenderTarget(RendererTarget::DepthLight);
        RHITexture* scene           = Renderer::getRenderTarget(RendererTarget::SceneHDR);

        cmdList->insertBarrier(
            scene->getImage(),
            scene->getFormat(),
            RHIImageLayout::General,
            RHIPipelineStage::FlagBits::TopOfPipe,
            RHIAccessUsage::FlagBits::Unknown,
            RHIPipelineStage::FlagBits::ComputeShader,
            RHIAccessUsage::FlagBits::ShaderStorageWrite);

        cmdList->setPipelineState(
            RHIPipelineStateBuilder()
                .setName("Light")
                .setType(RHIPipelineType::Compute)
                .addShader(Renderer::getShader(RendererShader::LightC))
                .build());

        std::array updates = {
            RHIDescriptorWrite{.reg      = 0, // t0
                               .resource = {gbufferAlbedo},
                               .type     = RHIDescriptorType::Texture},
            RHIDescriptorWrite{.reg      = 1, // t1
                               .resource = {gbufferNormal},
                               .type     = RHIDescriptorType::Texture},
            RHIDescriptorWrite{.reg      = 2, // t2
                               .resource = {gbufferMaterial},
                               .type     = RHIDescriptorType::Texture},
            RHIDescriptorWrite{.reg      = 3, // t3
                               .resource = {depthGBuffer},
                               .type     = RHIDescriptorType::Texture},
            RHIDescriptorWrite{.reg      = 4, // t4
                               .resource = {depthLight},
                               .type     = RHIDescriptorType::Texture},
            RHIDescriptorWrite{.reg      = 0, // u0
                               .resource = {scene},
                               .type     = RHIDescriptorType::TextureStorage},
        };
        cmdList->updateSpecificSet(updates);

        // static Matrix4 lightSpaceMatrix =
        //     math::projectionOrtho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 100.0f) *
        //     math::lookAt(
        //         Vector3(-0.3f, -1.0f, -0.5f),
        //         Vector3(0.0f, 0.0f, 0.0f),
        //         Vector3(0, 1, 0));
        // pushConstantData.setMatrix(lightSpaceMatrix);
        // cmdList->pushConstants(pushConstantData.asSpan());

        cmdList->dispatch(scene->getWidth() / 8, scene->getHeight() / 8, 1);
    }

    void Renderer::passDebugWireFrame(RHICommandList* cmdList, ecs::Resource<DrawcallStorage> drawcalls)
    {
        RHITexture* screenHDR = Renderer::getRenderTarget(RendererTarget::ScreenHDR);

        cmdList->setPipelineState(
            RHIPipelineStateBuilder()
                .setName("WireFrame")
                .setType(RHIPipelineType::Graphics)
                .setPrimitiveTopology(RHIPrimitiveTopology::TriangleList)
                .setRasterizerState(Renderer::getRasterizerState(RendererRasterizerState::Wireframe))
                .setDepthStencilState(Renderer::getDepthStencilState(RendererDepthStencilState::Off))
                .setBlendState(Renderer::getBlendState(RendererBlendState::Off))
                .addShader(Renderer::getShader(RendererShader::LineV))
                .addShader(Renderer::getShader(RendererShader::LineP))
                .setRenderTargetColorTexture(0, screenHDR) // 渲染到后处理之后
                .setScissor({0, 0, screenHDR->getWidth(), screenHDR->getHeight()})
                .setViewport(Renderer::getViewport())
                .build());

        for (Drawcall const& drawcall : drawcalls->solid)
        {
            if (Mesh* mesh = drawcall.mesh)
            {
                cmdList->setBufferVertex(mesh->getVertexBuffer());
                cmdList->setBufferIndex(mesh->getIndexBuffer());

                pushConstantData.setTransform(drawcall.transform);
                cmdList->pushConstants(pushConstantData.asSpan());

                cmdList->drawIndexed(mesh->getIndexBuffer()->getElementCount(), 0, 0, 0, 1);
            }
        }

        for (RenderObject const& object : drawcalls->ctx.opaqueObjects)
        {
            cmdList->setBufferVertex(object.mesh->getVertexBuffer());
            cmdList->setBufferIndex(object.mesh->getIndexBuffer());

            pushConstantData.setTransform(object.transform);
            cmdList->pushConstants(pushConstantData.asSpan());

            cmdList->drawIndexed(object.indexCount, object.startIndex, 0, 0, 1);
        }

        cmdList->renderPassEnd();
    }

    void Renderer::passBloom(RHICommandList* cmdList)
    {
        RHITexture* scene = Renderer::getRenderTarget(RendererTarget::SceneHDR);

        RHITexture* bloomInitial = Renderer::getRenderTarget(RendererTarget::BloomInitial);
        RHITexture* bloomStage0  = Renderer::getRenderTarget(RendererTarget::BloomDownSampleStage0);
        RHITexture* bloomStage1  = Renderer::getRenderTarget(RendererTarget::BloomDownSampleStage1);
        RHITexture* bloomStage2  = Renderer::getRenderTarget(RendererTarget::BloomDownSampleStage2);
        RHITexture* bloomStage3  = Renderer::getRenderTarget(RendererTarget::BloomDownSampleStage3);
        RHITexture* bloomFinal   = Renderer::getRenderTarget(RendererTarget::BloomFinal);

        // Downsample
        // Scene -> Stage0 | x2
        cmdList->insertBarrier(scene
                                   ->getImage(),
                               scene->getFormat(),
                               RHIImageLayout::ShaderRead,
                               RHIPipelineStage::FlagBits::FragmentShader,
                               RHIAccessUsage::FlagBits::ShaderWrite,
                               RHIPipelineStage::FlagBits::ComputeShader,
                               RHIAccessUsage::FlagBits::ShaderSampledRead);
        cmdList->insertBarrier(
            bloomInitial->getImage(),
            bloomInitial->getFormat(),
            RHIImageLayout::General,
            RHIPipelineStage::FlagBits::TopOfPipe,
            RHIAccessUsage::FlagBits::MemoryRead,
            RHIPipelineStage::FlagBits::ComputeShader,
            RHIAccessUsage::FlagBits::ShaderStorageWrite);

        cmdList->setPipelineState(
            RHIPipelineStateBuilder()
                .setName("BloomLuminance")
                .setType(RHIPipelineType::Compute)
                .addShader(Renderer::getShader(RendererShader::BloomLuminanceC))
                .build());

        std::array updatesBrightFilter = {
            RHIDescriptorWrite{.reg      = 0, // t0
                               .resource = {scene},
                               .type     = RHIDescriptorType::Texture},
            RHIDescriptorWrite{.reg      = 0, // u0
                               .resource = {bloomInitial},
                               .type     = RHIDescriptorType::TextureStorage},
        };
        cmdList->updateSpecificSet(updatesBrightFilter);
        cmdList->dispatch(bloomInitial->getWidth() / 8, bloomInitial->getHeight() / 8, 1);

        // Initial -> Stage0 | x2
        cmdList->blit(bloomInitial, bloomStage0);
        // Stage0 -> Stage1 | x4
        cmdList->blit(bloomStage0, bloomStage1);
        // Stage1 -> Stage2 | x8
        cmdList->blit(bloomStage1, bloomStage2);
        // Stage2 -> Stage3 | x16
        cmdList->blit(bloomStage2, bloomStage3);

        // Upsacle and Additive blend
        cmdList->insertBarrier(bloomStage0->getImage(), bloomStage0->getFormat(), RHIImageLayout::ShaderRead, RHIPipelineStage::FlagBits::Transfer, RHIAccessUsage::FlagBits::TransferRead, RHIPipelineStage::FlagBits::ComputeShader, RHIAccessUsage::FlagBits::ShaderSampledRead);
        cmdList->insertBarrier(bloomStage1->getImage(), bloomStage1->getFormat(), RHIImageLayout::ShaderRead, RHIPipelineStage::FlagBits::Transfer, RHIAccessUsage::FlagBits::TransferRead, RHIPipelineStage::FlagBits::ComputeShader, RHIAccessUsage::FlagBits::ShaderSampledRead);
        cmdList->insertBarrier(bloomStage2->getImage(), bloomStage2->getFormat(), RHIImageLayout::ShaderRead, RHIPipelineStage::FlagBits::Transfer, RHIAccessUsage::FlagBits::TransferRead, RHIPipelineStage::FlagBits::ComputeShader, RHIAccessUsage::FlagBits::ShaderSampledRead);
        cmdList->insertBarrier(bloomStage3->getImage(), bloomStage3->getFormat(), RHIImageLayout::ShaderRead, RHIPipelineStage::FlagBits::Transfer, RHIAccessUsage::FlagBits::TransferWrite, RHIPipelineStage::FlagBits::ComputeShader, RHIAccessUsage::FlagBits::ShaderSampledRead);
        cmdList->insertBarrier(bloomFinal->getImage(), bloomFinal->getFormat(), RHIImageLayout::General, RHIPipelineStage::FlagBits::TopOfPipe, RHIAccessUsage::FlagBits::Unknown, RHIPipelineStage::FlagBits::ComputeShader, RHIAccessUsage::FlagBits::ShaderStorageWrite);

        cmdList->setPipelineState(
            RHIPipelineStateBuilder()
                .setName("BloomUpscale")
                .setType(RHIPipelineType::Compute)
                .addShader(Renderer::getShader(RendererShader::BloomUpscaleC))
                .build());

        std::array updatesUpscale = {
            RHIDescriptorWrite{.reg      = 0, // t0
                               .resource = {bloomStage0},
                               .type     = RHIDescriptorType::Texture},
            RHIDescriptorWrite{.reg      = 1, // t1
                               .resource = {bloomStage1},
                               .type     = RHIDescriptorType::Texture},
            RHIDescriptorWrite{.reg      = 2, // t2
                               .resource = {bloomStage2},
                               .type     = RHIDescriptorType::Texture},
            RHIDescriptorWrite{.reg      = 3, // t3
                               .resource = {bloomStage3},
                               .type     = RHIDescriptorType::Texture},
            RHIDescriptorWrite{.reg      = 0, // u0
                               .resource = {bloomFinal},
                               .type     = RHIDescriptorType::TextureStorage},
        };
        cmdList->updateSpecificSet(updatesUpscale);
        cmdList->dispatch(bloomFinal->getWidth() / 8, bloomFinal->getHeight() / 8, 1);
    }

    void Renderer::passPostProcessing(RHICommandList* cmdList)
    {
        RHITexture* scene  = Renderer::getRenderTarget(RendererTarget::SceneHDR);
        RHITexture* bloom  = Renderer::getRenderTarget(RendererTarget::BloomFinal);
        RHITexture* screen = Renderer::getRenderTarget(RendererTarget::ScreenHDR);

        cmdList->insertBarrier(scene->getImage(), scene->getFormat(), RHIImageLayout::ShaderRead, RHIPipelineStage::FlagBits::FragmentShader, RHIAccessUsage::FlagBits::ShaderWrite, RHIPipelineStage::FlagBits::ComputeShader, RHIAccessUsage::FlagBits::ShaderSampledRead);
        cmdList->insertBarrier(bloom->getImage(), bloom->getFormat(), RHIImageLayout::ShaderRead, RHIPipelineStage::FlagBits::ComputeShader, RHIAccessUsage::FlagBits::ShaderStorageWrite, RHIPipelineStage::FlagBits::ComputeShader, RHIAccessUsage::FlagBits::ShaderSampledRead);
        cmdList->insertBarrier(screen->getImage(), screen->getFormat(), RHIImageLayout::General, RHIPipelineStage::FlagBits::TopOfPipe, RHIAccessUsage::FlagBits::MemoryRead, RHIPipelineStage::FlagBits::ComputeShader, RHIAccessUsage::FlagBits::ShaderStorageWrite);

        cmdList->setPipelineState(
            RHIPipelineStateBuilder()
                .setName("PostFX")
                .setType(RHIPipelineType::Compute)
                .addShader(Renderer::getShader(RendererShader::PostFXC))
                .build());

        std::array updates = {
            RHIDescriptorWrite{.reg      = 0, // t0
                               .resource = {scene},
                               .type     = RHIDescriptorType::Texture},
            RHIDescriptorWrite{.reg      = 1, // t1
                               .resource = {bloom},
                               .type     = RHIDescriptorType::Texture},
            RHIDescriptorWrite{.reg      = 0, // u0
                               .resource = {screen},
                               .type     = RHIDescriptorType::TextureStorage},
        };
        cmdList->updateSpecificSet(updates);

        cmdList->dispatch(screen->getWidth() / 8, screen->getHeight() / 8, 1);
    }

    void Renderer::passImGui(RHICommandList* cmdList)
    {
        cmdList->imguiPassBegin(
            Renderer::getRenderTarget(RendererTarget::ScreenHDR),
            math::Rectangle{
                0,
                0,
                static_cast<UInt>(Renderer::getResolutionOutput().x),
                static_cast<UInt>(Renderer::getResolutionOutput().y)});

        cmdList->imguiPassEnd(ImGui::GetDrawData());
    }

    void Renderer::produceFrame(
        RHICommandList* cmdList,
        ecs::Resource<GlobalContext> globalContext,
        ecs::Resource<DrawcallStorage> drawcalls,
        ecs::Resource<AssetServer> assetServer)
    {
        passDepthPrepass(cmdList, drawcalls);

        passShadowMap(cmdList, drawcalls);

        passGBuffer(cmdList, drawcalls, assetServer);

        passLight(cmdList);

        passBloom(cmdList);

        passPostProcessing(cmdList);

        if (globalContext->isWireFrameMode)
        {
            passDebugWireFrame(cmdList, drawcalls);
        }

        // passImGui(cmdList);

        drawcalls->ctx.clear();
    }

} // namespace Worse