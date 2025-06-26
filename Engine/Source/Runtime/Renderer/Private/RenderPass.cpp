#include "RHICommandList.hpp"
#include "RHIDevice.hpp"
#include "Descriptor/RHITexture.hpp"
#include "Pipeline/RHIPipelineState.hpp"
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

    void Renderer::passDpethPrepass(RHICommandList* cmdList)
    {
        // clang-format off
        cmdList->setPipelineState(
            RHIPipelineStateBuilder()
                .setName("DepthPrepass")
                .setType(RHIPipelineType::Graphics)
                .setPrimitiveTopology(RHIPrimitiveTopology::Trianglelist)
                .setRasterizerState(Renderer::getRasterizerState(RendererRasterizerState::Solid))
                .setDepthStencilState(Renderer::getDepthStencilState(RendererDepthStencilState::ReadWrite))
                .setBlendState(Renderer::getBlendState(RendererBlendState::Off))
                .addShader(Renderer::getShader(RendererShader::DepthPrepassV))
                .addShader(Renderer::getShader(RendererShader::DepthPrepassP))
                .setRenderTargetDepthTexture(Renderer::getRenderTarget(RendererTarget::Depth))
                .setScissor({0, 0, 800, 600})
                .setViewport(Renderer::getViewport())
                .setClearDepth(0.0f) // clear with far value
                .build());


        cmdList->setBufferVertex(Renderer::getStandardMesh(geometry::GeometryType::Cube)->getVertexBuffer());
        cmdList->setBufferIndex(Renderer::getStandardMesh(geometry::GeometryType::Cube)->getIndexBuffer());

        math::Matrix4 modelMatrix = math::Matrix4::IDENTITY();
        pushConstantData.setModel(modelMatrix);
        cmdList->pushConstants(pushConstantData.asSpan());

        cmdList->drawIndexed(Renderer::getStandardMesh(geometry::GeometryType::Cube)->getIndexBuffer()->getElementCount(), 0, 0, 0, 1);

        cmdList->renderPassEnd();
        // clang-format on
    }

    void Renderer::passTest(RHICommandList* cmdList)
    {
        // clang-format off

        std::array bindlessTextureupdates = {
            RHIDescriptorWrite{0,
                               0,
                               {Renderer::getTexture(RendererTexture::TestB)}},
        };

        RHIDevice::updateBindlessTextures(bindlessTextureupdates);

        cmdList->setPipelineState(
            RHIPipelineStateBuilder()
                .setName("testPso")
                .setType(RHIPipelineType::Graphics)
                .setPrimitiveTopology(RHIPrimitiveTopology::Trianglelist)
                .setRasterizerState(Renderer::getRasterizerState(RendererRasterizerState::Solid))
                .setDepthStencilState(Renderer::getDepthStencilState(RendererDepthStencilState::ReadEqual))
                .setBlendState(Renderer::getBlendState(RendererBlendState::Off))
                .addShader(Renderer::getShader(RendererShader::PlaceholderV))
                .addShader(Renderer::getShader(RendererShader::PlaceholderP))
                .setRenderTargetColorTexture(0, Renderer::getRenderTarget(RendererTarget::Render))
                .setRenderTargetDepthTexture(Renderer::getRenderTarget(RendererTarget::Depth))
                .setScissor({0, 0, 800, 600})
                .setViewport(Renderer::getViewport())
                .setClearColor(Color::Black())
                .build());

        cmdList->setBufferVertex(Renderer::getStandardMesh(geometry::GeometryType::Cube)->getVertexBuffer());
        cmdList->setBufferIndex(Renderer::getStandardMesh(geometry::GeometryType::Cube)->getIndexBuffer());

        math::Matrix4 modelMatrix = math::Matrix4::IDENTITY();
        pushConstantData.setModel(modelMatrix);
        cmdList->pushConstants(pushConstantData.asSpan());

        cmdList->drawIndexed(Renderer::getStandardMesh(geometry::GeometryType::Cube)->getIndexBuffer()->getElementCount(), 0, 0, 0, 1);

        cmdList->renderPassEnd();
        // clang-format on
    }

    void Renderer::passoPostProcessing(RHICommandList* cmdList)
    {
        // clang-format off
        RHITexture* frameRender = Renderer::getRenderTarget(RendererTarget::Render);
        RHITexture* frameOutput = Renderer::getRenderTarget(RendererTarget::Output);

        cmdList->insertBarrier(frameRender->getImage(), frameRender->getFormat(), RHIImageLayout::ShaderRead);

        cmdList->insertBarrier(frameOutput->getImage(), frameOutput->getFormat(), RHIImageLayout::General);

        cmdList->setPipelineState(
            RHIPipelineStateBuilder()
                .setName("computePso")
                .setType(RHIPipelineType::Compute)
                .addShader(Renderer::getShader(RendererShader::KuwaharaC))
                .build());

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

    void Renderer::produceFrame(RHICommandList* cmdList)
    {
        passDpethPrepass(cmdList);
        passTest(cmdList);
        passoPostProcessing(cmdList);
    }

} // namespace worse