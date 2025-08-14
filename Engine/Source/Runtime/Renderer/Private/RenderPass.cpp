#include "imgui.h"

#include "RHICommandList.hpp"
#include "RHITexture.hpp"
#include "RHIBuffer.hpp"
#include "Renderer.hpp"
#include "RendererBuffer.hpp"
namespace worse
{

    namespace
    {
        PushConstantData pushConstantData = {};
    }

    void Renderer::setPushParameters(f32 a, f32 b)
    {
        pushConstantData.setPadding(a, b);
    }

    void Renderer::passDpethPrepass(RHICommandList* cmdList, ecs::Resource<DrawcallStorage> drawcalls)
    {
        // clang-format off
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
                .setRenderTargetDepthTexture(Renderer::getRenderTarget(RendererTarget::Depth))
                .setScissor({0, 0, 1200, 720})
                .setViewport(Renderer::getViewport())
                .setClearDepth(0.0f) // clear with far value
                .build());

        for (Drawcall const& drawcall : drawcalls->solid)
        {
            if (Mesh* mesh = drawcall.mesh)
            {
                cmdList->setBufferVertex(mesh->getVertexBuffer());
                cmdList->setBufferIndex(mesh->getIndexBuffer());
    
                pushConstantData.setModel(drawcall.transform);
                cmdList->pushConstants(pushConstantData.asSpan());
    
                cmdList->drawIndexed(mesh->getIndexBuffer()->getElementCount(), 0, 0, 0, 1);
            }
        }

        for (RenderObject const& object : drawcalls->ctx.opaqueObjects)
        {
            cmdList->setBufferVertex(object.mesh->getVertexBuffer());
            cmdList->setBufferIndex(object.mesh->getIndexBuffer());

            pushConstantData.setModel(object.transform);
            cmdList->pushConstants(pushConstantData.asSpan());
            
            cmdList->drawIndexed(object.indexCount, object.startIndex, 0, 0, 1);
        }

        cmdList->renderPassEnd();
        // clang-format on
    }

    void Renderer::passColor(RHICommandList* cmdList, ecs::Resource<DrawcallStorage> drawcalls, ecs::Resource<AssetServer> assetServer)
    {
        cmdList->setPipelineState(
            RHIPipelineStateBuilder()
                .setName("PBR")
                .setType(RHIPipelineType::Graphics)
                .setPrimitiveTopology(RHIPrimitiveTopology::TriangleList)
                .setRasterizerState(Renderer::getRasterizerState(RendererRasterizerState::Solid))
                .setDepthStencilState(Renderer::getDepthStencilState(RendererDepthStencilState::ReadGreaterEqual))
                .setBlendState(Renderer::getBlendState(RendererBlendState::Off))
                .addShader(Renderer::getShader(RendererShader::PBRV))
                .addShader(Renderer::getShader(RendererShader::PBRP))
                .setRenderTargetColorTexture(0, Renderer::getRenderTarget(RendererTarget::SceneHDR))
                .setRenderTargetColorTexture(1, Renderer::getRenderTarget(RendererTarget::GBufferNormal))
                .setRenderTargetColorTexture(2, Renderer::getRenderTarget(RendererTarget::GBufferAlbedo))
                .setRenderTargetDepthTexture(Renderer::getRenderTarget(RendererTarget::Depth))
                .setScissor({0, 0, 1200, 720})
                .setViewport(Renderer::getViewport())
                .setClearColor(Color{0.02f, 0.02f, 0.02f, 1.0f})
                .setClearDepth(2.0f)
                .build());

        // 材质缓冲
        std::array updates = {
            RHIDescriptorWrite{.reg      = 0, // t0
                               .resource = {Renderer::getMaterialBuffer()},
                               .type     = RHIDescriptorType::StructuredBuffer},
        };
        cmdList->updateSpecificSet(updates);

        for (Drawcall const& drawcall : drawcalls->solid)
        {
            if (Mesh* mesh = drawcall.mesh)
            {
                cmdList->setBufferVertex(mesh->getVertexBuffer());
                cmdList->setBufferIndex(mesh->getIndexBuffer());

                pushConstantData.setModel(drawcall.transform);
                pushConstantData.setMaterialId(drawcall.materialIndex);
                cmdList->pushConstants(pushConstantData.asSpan());

                cmdList->drawIndexed(mesh->getIndexBuffer()->getElementCount(), 0, 0, 0, 1);
            }
        }

        for (RenderObject const& object : drawcalls->ctx.opaqueObjects)
        {
            cmdList->setBufferVertex(object.mesh->getVertexBuffer());
            cmdList->setBufferIndex(object.mesh->getIndexBuffer());

            pushConstantData.setModel(object.transform);
            pushConstantData.setMaterialId(assetServer->getMaterialIndex(object.material));

            cmdList->pushConstants(pushConstantData.asSpan());
            cmdList->drawIndexed(object.indexCount, object.startIndex, 0, 0, 1);
        }

        cmdList->setPipelineState(
            RHIPipelineStateBuilder()
                .setName("Point")
                .setType(RHIPipelineType::Graphics)
                .setPrimitiveTopology(RHIPrimitiveTopology::PointList)
                .setRasterizerState(Renderer::getRasterizerState(RendererRasterizerState::Solid))
                .setDepthStencilState(Renderer::getDepthStencilState(RendererDepthStencilState::ReadGreaterEqual))
                .setBlendState(Renderer::getBlendState(RendererBlendState::Off))
                .addShader(Renderer::getShader(RendererShader::PointV))
                .addShader(Renderer::getShader(RendererShader::PointP))
                .setRenderTargetColorTexture(0, Renderer::getRenderTarget(RendererTarget::SceneHDR))
                .setRenderTargetDepthTexture(Renderer::getRenderTarget(RendererTarget::Depth))
                .setScissor({0, 0, 1200, 720})
                .setViewport(Renderer::getViewport())
                .setClearDepth(2.0f)
                .build());

        for (Drawcall const& drawcall : drawcalls->point)
        {
            if (Mesh* mesh = drawcall.mesh)
            {
                cmdList->setBufferVertex(mesh->getVertexBuffer());

                pushConstantData.setModel(drawcall.transform);
                pushConstantData.setMaterialId(drawcall.materialIndex);

                pushConstantData.setPadding(1.0f, 0.0f); // Point size
                cmdList->pushConstants(pushConstantData.asSpan());
                cmdList->draw(mesh->getVertexBuffer()->getElementCount());
            }
        }

        cmdList->renderPassEnd();
    }

    void Renderer::passWireFrame(RHICommandList* cmdList, ecs::Resource<DrawcallStorage> drawcalls)
    {
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
                .setRenderTargetColorTexture(0, Renderer::getRenderTarget(RendererTarget::ScreenHDR)) // 渲染到后处理之后
                .setScissor({0, 0, 1200, 720})
                .setViewport(Renderer::getViewport())
                .build());

        for (Drawcall const& drawcall : drawcalls->solid)
        {
            if (Mesh* mesh = drawcall.mesh)
            {
                cmdList->setBufferVertex(mesh->getVertexBuffer());
                cmdList->setBufferIndex(mesh->getIndexBuffer());

                pushConstantData.setModel(drawcall.transform);
                cmdList->pushConstants(pushConstantData.asSpan());

                cmdList->drawIndexed(mesh->getIndexBuffer()->getElementCount(), 0, 0, 0, 1);
            }
        }

        for (RenderObject const& object : drawcalls->ctx.opaqueObjects)
        {
            cmdList->setBufferVertex(object.mesh->getVertexBuffer());
            cmdList->setBufferIndex(object.mesh->getIndexBuffer());

            pushConstantData.setModel(object.transform);

            cmdList->pushConstants(pushConstantData.asSpan());
            cmdList->drawIndexed(object.indexCount, object.startIndex, 0, 0, 1);
        }

        cmdList->renderPassEnd();
    }

    void Renderer::passBloom(RHICommandList* cmdList)
    {
        RHITexture* scene  = Renderer::getRenderTarget(RendererTarget::SceneHDR);
        RHITexture* stage0 = Renderer::getRenderTarget(RendererTarget::BloomDownSampleStage0);
        RHITexture* stage1 = Renderer::getRenderTarget(RendererTarget::BloomDownSampleStage1);
        RHITexture* stage2 = Renderer::getRenderTarget(RendererTarget::BloomDownSampleStage2);
        RHITexture* stage3 = Renderer::getRenderTarget(RendererTarget::BloomDownSampleStage3);
        RHITexture* bloom  = Renderer::getRenderTarget(RendererTarget::BloomFinal);

        // Downsample
        // Scene(Luminance filter) -> Stage0 | x0.5  
        cmdList->insertBarrier(scene->getImage(), scene->getFormat(), RHIImageLayout::ShaderRead);
        cmdList->insertBarrier(stage0->getImage(), stage0->getFormat(), RHIImageLayout::General);
        cmdList->setPipelineState(
            RHIPipelineStateBuilder()
                .setName("BloomBrightFilter")
                .setType(RHIPipelineType::Compute)
                .addShader(Renderer::getShader(RendererShader::BloomBrightFilterC))
                .build());

        std::array updatesBrightFilter = {
            RHIDescriptorWrite{.reg      = 0, // t0
                               .resource = {scene},
                               .type     = RHIDescriptorType::Texture},
            RHIDescriptorWrite{.reg      = 0, // u0
                               .resource = {stage0},
                               .type     = RHIDescriptorType::TextureStorage},
        };
        cmdList->updateSpecificSet(updatesBrightFilter);
        pushConstantData.setF4(math::Vector4::ZERO());
        pushConstantData.setF2(math::Vector2(1.0f, 1.5f));
        cmdList->pushConstants(pushConstantData.asSpan());
        cmdList->dispatch(stage0->getWidth() / 8, stage0->getHeight() / 8, 1);

        // Satge0 -> Stage1 | x2
        cmdList->insertBarrier(stage0->getImage(), stage0->getFormat(), RHIImageLayout::ShaderRead);
        cmdList->insertBarrier(stage1->getImage(), stage1->getFormat(), RHIImageLayout::General);
        // cmdList->setPipelineState(
        //     RHIPipelineStateBuilder()
        //         .setName("BloomDownSample")
        //         .setType(RHIPipelineType::Compute)
        //         .addShader(Renderer::getShader(RendererShader::BloomDownSampleC))
        //         .build());

        // std::array updatesDownSample = {
        //     RHIDescriptorWrite{.reg      = 0, // t0
        //                        .resource = {stage0},
        //                        .type     = RHIDescriptorType::Texture},
        //     RHIDescriptorWrite{.reg      = 0, // u0
        //                        .resource = {stage1},
        //                        .type     = RHIDescriptorType::TextureStorage},
        // };
        // cmdList->updateSpecificSet(updatesDownSample);
        // pushConstantData.setF4(math::Vector4::ZERO());
        // cmdList->pushConstants(pushConstantData.asSpan());
        // cmdList->dispatch(stage1->getWidth() / 8, stage1->getHeight() / 8, 1);
        cmdList->blit(stage0, stage1);

        // Stage1 -> Stage2 | x4
        cmdList->insertBarrier(stage1->getImage(), stage1->getFormat(), RHIImageLayout::ShaderRead);
        cmdList->insertBarrier(stage2->getImage(), stage2->getFormat(), RHIImageLayout::General);
        cmdList->blit(stage1, stage2);

        // Stage2 -> Stage3 | x8
        cmdList->insertBarrier(stage2->getImage(), stage2->getFormat(), RHIImageLayout::ShaderRead);
        cmdList->insertBarrier(stage3->getImage(), stage3->getFormat(), RHIImageLayout::General);
        cmdList->blit(stage2, stage3);

        // Upsacle and Additive blend
        cmdList->insertBarrier(stage0->getImage(), stage0->getFormat(), RHIImageLayout::ShaderRead);
        cmdList->insertBarrier(stage1->getImage(), stage1->getFormat(), RHIImageLayout::ShaderRead);
        cmdList->insertBarrier(stage2->getImage(), stage2->getFormat(), RHIImageLayout::ShaderRead);
        cmdList->insertBarrier(stage3->getImage(), stage3->getFormat(), RHIImageLayout::ShaderRead);
        cmdList->insertBarrier(bloom->getImage(), bloom->getFormat(), RHIImageLayout::General);

        cmdList->setPipelineState(
            RHIPipelineStateBuilder()
                .setName("BloomUpscale")
                .setType(RHIPipelineType::Compute)
                .addShader(Renderer::getShader(RendererShader::BloomUpscaleC))
                .build());

        std::array updatesUpscale = {
            RHIDescriptorWrite{.reg      = 0, // t0
                               .resource = {stage0},
                               .type     = RHIDescriptorType::Texture},
            RHIDescriptorWrite{.reg      = 1, // t1
                               .resource = {stage1},
                               .type     = RHIDescriptorType::Texture},
            RHIDescriptorWrite{.reg      = 2, // t2
                               .resource = {stage2},
                               .type     = RHIDescriptorType::Texture},
            RHIDescriptorWrite{.reg      = 3, // t3
                               .resource = {stage3},
                               .type     = RHIDescriptorType::Texture},
            RHIDescriptorWrite{.reg      = 0, // u0
                               .resource = {bloom},
                               .type     = RHIDescriptorType::TextureStorage},
        };
        cmdList->updateSpecificSet(updatesUpscale);
        cmdList->pushConstants(pushConstantData.asSpan());
        cmdList->dispatch(bloom->getWidth() / 8, bloom->getHeight() / 8, 1);
    }

    void Renderer::passPostProcessing(RHICommandList* cmdList)
    {
        RHITexture* scene  = Renderer::getRenderTarget(RendererTarget::SceneHDR);
        RHITexture* bloom  = Renderer::getRenderTarget(RendererTarget::BloomFinal);
        RHITexture* screen = Renderer::getRenderTarget(RendererTarget::ScreenHDR);

        // 同步
        cmdList->insertBarrier(scene->getImage(), scene->getFormat(), RHIImageLayout::ShaderRead);
        cmdList->insertBarrier(bloom->getImage(), bloom->getFormat(), RHIImageLayout::ShaderRead);
        cmdList->insertBarrier(screen->getImage(), screen->getFormat(), RHIImageLayout::General);

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

        cmdList->pushConstants(pushConstantData.asSpan());

        math::Vector2 resolutionOutput = Renderer::getResolutionOutput();
        cmdList->dispatch(resolutionOutput.x / 8, resolutionOutput.y / 8, 1);
    }

    void Renderer::passImGui(RHICommandList* cmdList)
    {
        cmdList->imguiPassBegin(
            Renderer::getRenderTarget(RendererTarget::ScreenHDR),
            math::Rectangle{
                0,
                0,
                static_cast<u32>(Renderer::getResolutionOutput().x),
                static_cast<u32>(Renderer::getResolutionOutput().y)});

        cmdList->imguiPassEnd(ImGui::GetDrawData());
    }

    void Renderer::produceFrame(RHICommandList* cmdList,
                                ecs::Resource<GlobalContext> globalContext,
                                ecs::Resource<DrawcallStorage> drawcalls,
                                ecs::Resource<AssetServer> assetServer)
    {
        passDpethPrepass(cmdList, drawcalls);

        passColor(cmdList, drawcalls, assetServer);

        passBloom(cmdList);

        passPostProcessing(cmdList);

        if (globalContext->isWireFrameMode)
        {
            passWireFrame(cmdList, drawcalls);
        }

        // passImGui(cmdList);

        drawcalls->ctx.clear();
    }

} // namespace worse