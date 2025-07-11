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

    void Renderer::setPushParameters(float a, float b)
    {
        pushConstantData.setPadding(a, b);
    }

    void Renderer::passDpethPrepass(RHICommandList* cmdList,
                                    ecs::Resource<DrawcallStorage> drawcalls,
                                    ecs::ResourceArray<Mesh> meshes)
    {
        // clang-format off
        cmdList->setPipelineState(Renderer::getPipelineState(RendererPSO::DepthPrepass));

        for (Drawcall const& drawcall : drawcalls->solid)
        {
            Mesh* mesh = meshes.get(drawcall.meshIndex);

            cmdList->setBufferVertex(mesh->getVertexBuffer());
            cmdList->setBufferIndex(mesh->getIndexBuffer());

            pushConstantData.setModel(drawcall.transform);
            cmdList->pushConstants(pushConstantData.asSpan());

            cmdList->drawIndexed(mesh->getIndexBuffer()->getElementCount(), 0, 0, 0, 1);
        }

        cmdList->renderPassEnd();
        // clang-format on
    }

    void Renderer::passColor(RHICommandList* cmdList,
                             ecs::Resource<DrawcallStorage> drawcalls,
                             ecs::ResourceArray<Mesh> meshes)
    {
        // clang-format off
        cmdList->setPipelineState(Renderer::getPipelineState(RendererPSO::PBR));

        std::array updates = {
            RHIDescriptorWrite{.reg      = 0, // t0
                               .resource = {Renderer::getMaterialBuffer()},
                               .type     = RHIDescriptorType::StructuredBuffer},
        };
        cmdList->updateSpecificSet(updates);

        for (Drawcall const& drawcall : drawcalls->solid)
        {   
            Mesh* mesh = meshes.get(drawcall.meshIndex);

            cmdList->setBufferVertex(mesh->getVertexBuffer());
            cmdList->setBufferIndex(mesh->getIndexBuffer());

            pushConstantData.setModel(drawcall.transform);
            pushConstantData.setMaterialId(drawcall.materialIndex);

            cmdList->pushConstants(pushConstantData.asSpan());
            cmdList->drawIndexed(mesh->getIndexBuffer()->getElementCount(), 0, 0, 0, 1);
        }

        cmdList->setPipelineState(Renderer::getPipelineState(RendererPSO::Point));

        for (Drawcall const& drawcall : drawcalls->point)
        {   
            Mesh* mesh = meshes.get(drawcall.meshIndex);

            cmdList->setBufferVertex(mesh->getVertexBuffer());
            // cmdList->setBufferIndex(mesh->getIndexBuffer());

            pushConstantData.setModel(drawcall.transform);
            pushConstantData.setMaterialId(drawcall.materialIndex);

            pushConstantData.setPadding(1.0f, 0.0f); // Point size
            cmdList->pushConstants(pushConstantData.asSpan());
            // cmdList->drawIndexed(mesh->getIndexBuffer()->getElementCount(), 0, 0, 0, 1);
            cmdList->draw(mesh->getVertexBuffer()->getElementCount());
        }

        cmdList->renderPassEnd();
        // clang-format on
    }

    void Renderer::passWireFrame(RHICommandList* cmdList,
                                 ecs::Resource<DrawcallStorage> drawcalls,
                                 ecs::ResourceArray<Mesh> meshes)
    {
        // clang-format off
        cmdList->setPipelineState(Renderer::getPipelineState(RendererPSO::Wireframe));

        for (Drawcall const& drawcall : drawcalls->solid)
        {
            Mesh* mesh = meshes.get(drawcall.meshIndex);

            cmdList->setBufferVertex(mesh->getVertexBuffer());
            cmdList->setBufferIndex(mesh->getIndexBuffer());

            pushConstantData.setModel(drawcall.transform);
            cmdList->pushConstants(pushConstantData.asSpan());

            cmdList->drawIndexed(mesh->getIndexBuffer()->getElementCount(), 0, 0, 0, 1);
        }

        cmdList->renderPassEnd();
        // clang-format on
    }

    void Renderer::passPostProcessing(RHICommandList* cmdList)
    {
        // clang-format off
        RHITexture* frameRender = Renderer::getRenderTarget(RendererTarget::Render);
        RHITexture* frameOutput = Renderer::getRenderTarget(RendererTarget::Output);

        cmdList->insertBarrier(frameRender->getImage(), frameRender->getFormat(), RHIImageLayout::ShaderRead);
        cmdList->insertBarrier(frameOutput->getImage(), frameOutput->getFormat(), RHIImageLayout::General);

        cmdList->setPipelineState(Renderer::getPipelineState(RendererPSO::PostProcessing));

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
                static_cast<std::uint32_t>(Renderer::getResolutionOutput().x),
                static_cast<std::uint32_t>(Renderer::getResolutionOutput().y)});

        cmdList->imguiPassEnd(ImGui::GetDrawData());
    }

    void Renderer::produceFrame(RHICommandList* cmdList,
                                ecs::Resource<GlobalContext> globalContext,
                                ecs::Resource<DrawcallStorage> drawcalls,
                                ecs::ResourceArray<Mesh> meshes,
                                ecs::ResourceArray<TextureWrite> textureWrites)
    {
        Renderer::writeBindlessTextures(textureWrites);

        passDpethPrepass(cmdList, drawcalls, meshes);

        Renderer::getRenderTarget(RendererTarget::Depth)
            ->convertImageLayout(cmdList, RHIImageLayout::DepthReadOnly);

        passColor(cmdList, drawcalls, meshes);

        if (globalContext->isWireFrameMode)
        {
            passWireFrame(cmdList, drawcalls, meshes);
        }

        passPostProcessing(cmdList);

        passImGui(cmdList);
    }

} // namespace worse