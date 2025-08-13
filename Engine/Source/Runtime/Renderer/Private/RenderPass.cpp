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
        cmdList->setPipelineState(Renderer::getPipelineState(RendererPSOType::DepthPrepass));

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

        for (auto const& object : drawcalls->ctx.opaqueObjects)
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
        cmdList->setPipelineState(Renderer::getPipelineState(RendererPSOType::PBR));

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

        cmdList->setPipelineState(Renderer::getPipelineState(RendererPSOType::Point));

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
        cmdList->setPipelineState(Renderer::getPipelineState(RendererPSOType::Wireframe));

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

        for (auto const& object : drawcalls->ctx.opaqueObjects)
        {
            cmdList->setBufferVertex(object.mesh->getVertexBuffer());
            cmdList->setBufferIndex(object.mesh->getIndexBuffer());

            pushConstantData.setModel(object.transform);

            cmdList->pushConstants(pushConstantData.asSpan());
            cmdList->drawIndexed(object.indexCount, object.startIndex, 0, 0, 1);
        }

        cmdList->renderPassEnd();
    }

    void Renderer::passPostProcessing(RHICommandList* cmdList)
    {
        // clang-format off
        RHITexture* frameRender = Renderer::getRenderTarget(RendererTarget::Render);
        RHITexture* frameOutput = Renderer::getRenderTarget(RendererTarget::Output);

        // 保证渲染完成
        cmdList->insertBarrier(frameRender->getImage(), frameRender->getFormat(), RHIImageLayout::ShaderRead);
        // 转换为通用布局以便计算着色器写入
        cmdList->insertBarrier(frameOutput->getImage(), frameOutput->getFormat(), RHIImageLayout::General);

        cmdList->setPipelineState(Renderer::getPipelineState(RendererPSOType::PostFX));

        std::array updates = {
            RHIDescriptorWrite{.reg      = 0, // t0
                               .resource = {frameRender},
                               .type     = RHIDescriptorType::Texture},
            RHIDescriptorWrite{.reg      = 0, // u0
                               .resource = {frameOutput},
                               .type     = RHIDescriptorType::TextureStorage},
        };
        cmdList->updateSpecificSet(updates);

        cmdList->pushConstants(pushConstantData.asSpan());

        math::Vector2 resolutionOutput = Renderer::getResolutionOutput();
        cmdList->dispatch(resolutionOutput.x / 8, resolutionOutput.y / 8, 1);
        // clang-format on
    }

    void Renderer::passImGui(RHICommandList* cmdList)
    {
        cmdList->imguiPassBegin(
            Renderer::getRenderTarget(RendererTarget::Output),
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

        passPostProcessing(cmdList);

        if (globalContext->isWireFrameMode)
        {
            passWireFrame(cmdList, drawcalls);
        }

        passImGui(cmdList);

        drawcalls->ctx.clear();
    }

} // namespace worse