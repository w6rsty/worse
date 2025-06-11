#include "Math/Vector2.hpp"
#include "Renderer.hpp"
#include "RHIShader.hpp"
#include "RHIVertex.hpp"
#include "Pipeline/RHIDepthStencilState.hpp"
#include "Pipeline/RHIRasterizerState.hpp"
#include "Pipeline/RHIBlendState.hpp"
#include "Descriptor/RHITexture.hpp"

#include <filesystem>
#include <memory>

namespace worse
{
    namespace
    {
        // clang-format off
        std::array<std::shared_ptr<RHIRasterizerState>, k_rendererRasterizerStateCount>
            rasterizerStates;
        std::array<std::shared_ptr<RHIDepthStencilState>, k_rendererDepthStencilStateCount>
            depthStencilStates;
        std::array<std::shared_ptr<RHIBlendState>, k_rendererBlendStateCount>
            blendStates;
        
        std::array<std::shared_ptr<RHITexture>, k_rendererTargetCount>
            renderTargets;
        
        std::array<std::shared_ptr<RHIShader>, k_rendererShaderCount>
            shaders;
        // clang-format on
    } // namespace

    void Renderer::createRasterizerStates()
    {
        rasterizerStates[static_cast<std::size_t>(
            RendererRasterizerState::Solid)] =
            std::make_shared<RHIRasterizerState>(RHIPolygonMode::Solid,
                                                 RHICullMode::None,
                                                 RHIFrontFace::CW,
                                                 1.0f,
                                                 false,
                                                 0.0f,
                                                 0.0f,
                                                 0.0f);

        rasterizerStates[static_cast<std::size_t>(
            RendererRasterizerState::Wireframe)] =
            std::make_shared<RHIRasterizerState>(RHIPolygonMode::Wirefame,
                                                 RHICullMode::None,
                                                 RHIFrontFace::CW,
                                                 1.0f,
                                                 false,
                                                 0.0f,
                                                 0.0f,
                                                 0.0f);
    }

    void Renderer::createDepthStencilStates()
    {
        depthStencilStates[static_cast<std::size_t>(
            RendererDepthStencilState::Off)] =
            std::make_shared<RHIDepthStencilState>(false,
                                                   false,
                                                   RHICompareOperation::Less,
                                                   false,
                                                   false,
                                                   RHICompareOperation::Always,
                                                   RHIStencilOperation::Keep,
                                                   RHIStencilOperation::Keep,
                                                   RHIStencilOperation::Replace,
                                                   0x1,
                                                   0x1);
    }

    void Renderer::createBlendStates()
    {
        blendStates[static_cast<std::size_t>(RendererBlendState::Off)] =
            std::make_shared<RHIBlendState>(false,
                                            RHIBlendFactor::SrcAlpha,
                                            RHIBlendFactor::OneMinusSrcAlpha,
                                            RHIBlendOperation::Add,
                                            RHIBlendFactor::One,
                                            RHIBlendFactor::One,
                                            RHIBlendOperation::Add,
                                            1.0f);
    }

    void Renderer::createShaders()
    {
        std::filesystem::path resourceRoot = "/Users/w6rsty/dev/Cpp/worse";
        std::filesystem::path shaderPath = resourceRoot / "Engine" / "Shaders";

        // clang-format off
        shaders[static_cast<std::size_t>(RendererShader::QuadV)] = std::make_shared<RHIShader>("quard_vertex");
        shaders[static_cast<std::size_t>(RendererShader::QuadV)]->compile(shaderPath / "quad.hlsl", RHIShaderType::Vertex, RHIVertexType::Pos);


        shaders[static_cast<std::size_t>(RendererShader::QuadP)] = std::make_shared<RHIShader>("quard_pixel");
        shaders[static_cast<std::size_t>(RendererShader::QuadP)]->compile(shaderPath / "quad.hlsl", RHIShaderType::Pixel);
        // clang-format on
    }

    void Renderer::createRendererTarget()
    {
        // clang-format off
        math::Vector2 renderResolution = getResolutionRender();
        std::uint32_t renderWidth  = static_cast<std::uint32_t>(renderResolution.x);
        std::uint32_t renderHeight = static_cast<std::uint32_t>(renderResolution.y);

        RHIFormat standardFormat = RHIFormat::B8R8G8A8Unorm;

        renderTargets[static_cast<std::size_t>(RendererTarget::Render)] = std::make_shared<RHITexture>(
            RHITextureType::Texture2D,
            renderWidth, renderHeight, 1,
            standardFormat,
            RHITextureUsageFlagBits::Rtv | RHITextureUsageFlagBits::Uav | RHITextureUsageFlagBits::Srv | RHITextureUsageFlagBits::ClearOrBlit,
            "render");

        renderTargets[static_cast<std::size_t>(RendererTarget::Output)] = std::make_shared<RHITexture>(
            RHITextureType::Texture2D,
            renderWidth, renderHeight, 1,
            standardFormat,
            RHITextureUsageFlagBits::Rtv | RHITextureUsageFlagBits::Uav | RHITextureUsageFlagBits::Srv | RHITextureUsageFlagBits::ClearOrBlit,
            "output");
        // clang-format on
    }

    void Renderer::destroyResources()
    {
        rasterizerStates.fill(nullptr);
        depthStencilStates.fill(nullptr);
        blendStates.fill(nullptr);
        renderTargets.fill(nullptr);
        shaders.fill(nullptr);
    }

    RHIRasterizerState*
    Renderer::getRasterizerState(RendererRasterizerState const state)
    {
        return rasterizerStates[static_cast<std::size_t>(state)].get();
    }

    RHIDepthStencilState*
    Renderer::getDepthStencilState(RendererDepthStencilState const state)
    {
        return depthStencilStates[static_cast<std::size_t>(state)].get();
    }

    RHIBlendState* Renderer::getBlendState(RendererBlendState const state)
    {
        return blendStates[static_cast<std::size_t>(state)].get();
    }

    RHITexture* Renderer::getRenderTarget(RendererTarget const target)
    {
        return renderTargets[static_cast<std::size_t>(target)].get();
    }

    RHIShader* Renderer::getShader(RendererShader const shader)
    {
        return shaders[static_cast<std::size_t>(shader)].get();
    }

} // namespace worse
