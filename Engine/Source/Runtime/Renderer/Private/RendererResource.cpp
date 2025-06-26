#include "Geometry/GeometryGeneration.hpp"
#include "Math/Vector.hpp"
#include "Mesh.hpp"
#include "Types.hpp"
#include "Renderer.hpp"
#include "RHIShader.hpp"
#include "RHIVertex.hpp"
#include "Pipeline/RHIDepthStencilState.hpp"
#include "Pipeline/RHIRasterizerState.hpp"
#include "Pipeline/RHIBlendState.hpp"
#include "Descriptor/RHITexture.hpp"
#include "Descriptor/RHISampler.hpp"

#include <filesystem>
#include <memory>

namespace worse
{
    namespace
    {
        // clang-format off
        EnumArray<RendererRasterizerState, std::shared_ptr<RHIRasterizerState>>     rasterizerStates;
        EnumArray<RendererDepthStencilState, std::shared_ptr<RHIDepthStencilState>> depthStencilStates;
        EnumArray<RendererBlendState, std::shared_ptr<RHIBlendState>>               blendStates;
        EnumArray<RendererTarget, std::shared_ptr<RHITexture>>                      renderTargets;
        EnumArray<RendererShader, std::shared_ptr<RHIShader>>                       shaders;
        EnumArray<RendererTexture, std::shared_ptr<RHITexture>>                     textures;
        EnumArray<RHISamplerType, std::shared_ptr<RHISampler>>                      samplers;

        EnumArray<geometry::GeometryType, std::shared_ptr<Mesh>> standardMeshes;
        // clang-format on
    } // namespace

    void Renderer::createRasterizerStates()
    {
        // clang-format off
        rasterizerStates[RendererRasterizerState::Solid]     = std::make_shared<RHIRasterizerState>(RHIPolygonMode::Solid, RHICullMode::None, RHIFrontFace::CCW, 1.0f, false, 0.0f, 0.0f, 0.0f);
        rasterizerStates[RendererRasterizerState::Wireframe] = std::make_shared<RHIRasterizerState>(RHIPolygonMode::Wirefame, RHICullMode::None, RHIFrontFace::CW, 1.0f, false, 0.0f, 0.0f, 0.0f);
        // clang-format on
    }

    void Renderer::createDepthStencilStates()
    {
        // clang-format off
        //                                                                                                test  | write  | compare
        depthStencilStates[RendererDepthStencilState::Off]       = std::make_shared<RHIDepthStencilState>(false,  false,   RHICompareOperation::Never);
        depthStencilStates[RendererDepthStencilState::ReadWrite] = std::make_shared<RHIDepthStencilState>(true,   true,    RHICompareOperation::GreaterEqual);
        depthStencilStates[RendererDepthStencilState::ReadEqual] = std::make_shared<RHIDepthStencilState>(true,   false,   RHICompareOperation::Equal);
        // clang-format on
    }

    void Renderer::createBlendStates()
    {
        // clang-format off
        blendStates[RendererBlendState::Off] = std::make_shared<RHIBlendState>(false, RHIBlendFactor::SrcAlpha, RHIBlendFactor::OneMinusSrcAlpha, RHIBlendOperation::Add, RHIBlendFactor::One, RHIBlendFactor::One, RHIBlendOperation::Add, 1.0f);
        // clang-format on
    }

    void Renderer::createRendererTarget()
    {
        // clang-format off
        math::Vector2 renderResolution = getResolutionRender();
        std::uint32_t renderWidth  = static_cast<std::uint32_t>(renderResolution.x);
        std::uint32_t renderHeight = static_cast<std::uint32_t>(renderResolution.y);

        RHIFormat standardFormat = RHIFormat::B8R8G8A8Unorm;

        std::vector<RHITextureSlice> dummy;
        renderTargets[RendererTarget::Render] = std::make_shared<RHITexture>(RHITextureType::Texture2D, renderWidth, renderHeight, 1, 1, standardFormat, RHITextureViewUsageFlagBits::Rtv | RHITextureViewUsageFlagBits::Uav | RHITextureViewUsageFlagBits::Srv | RHITextureViewUsageFlagBits::ClearOrBlit, dummy, "render");
        renderTargets[RendererTarget::Output] = std::make_shared<RHITexture>(RHITextureType::Texture2D, renderWidth, renderHeight, 1, 1, standardFormat, RHITextureViewUsageFlagBits::Rtv | RHITextureViewUsageFlagBits::Uav | RHITextureViewUsageFlagBits::Srv | RHITextureViewUsageFlagBits::ClearOrBlit, dummy, "output");
        renderTargets[RendererTarget::Depth]  = std::make_shared<RHITexture>(RHITextureType::Texture2D, renderWidth, renderHeight, 1, 1, RHIFormat::D32Float, RHITextureViewUsageFlagBits::Dsv | RHITextureViewUsageFlagBits::Srv | RHITextureViewUsageFlagBits::ClearOrBlit, dummy, "depth");
        // clang-format on
    }

    void Renderer::createShaders()
    {
        std::filesystem::path resourceRoot = "/Users/w6rsty/dev/Cpp/worse";
        std::filesystem::path shaderPath = resourceRoot / "Engine" / "Shaders";

        // clang-format off
        shaders[RendererShader::PlaceholderV] = std::make_shared<RHIShader>("PlaceholderV");
        shaders[RendererShader::PlaceholderV]->compile(shaderPath / "Placeholder.hlsl", RHIShaderType::Vertex, RHIVertexType::PosUvNrmTan);
        shaders[RendererShader::PlaceholderP] = std::make_shared<RHIShader>("PlaceholderP");
        shaders[RendererShader::PlaceholderP]->compile(shaderPath / "Placeholder.hlsl", RHIShaderType::Pixel);

        shaders[RendererShader::DepthPrepassV] = std::make_shared<RHIShader>("DepthPrepassV");
        shaders[RendererShader::DepthPrepassV]->compile(shaderPath / "DepthPrepass.hlsl", RHIShaderType::Vertex, RHIVertexType::PosUvNrmTan);
        shaders[RendererShader::DepthPrepassP] = std::make_shared<RHIShader>("DepthPrepassP");
        shaders[RendererShader::DepthPrepassP]->compile(shaderPath / "DepthPrepass.hlsl", RHIShaderType::Pixel);

        shaders[RendererShader::KuwaharaC] = std::make_shared<RHIShader>("KuwaharaC");
        shaders[RendererShader::KuwaharaC]->compile(shaderPath / "Kuwahara.hlsl", RHIShaderType::Compute);

        shaders[RendererShader::PBRV] = std::make_shared<RHIShader>("PBRV");
        shaders[RendererShader::PBRV]->compile(shaderPath / "PBR.hlsl", RHIShaderType::Vertex, RHIVertexType::PosUvNrmTan);
        shaders[RendererShader::PBRP] = std::make_shared<RHIShader>("PBRP");
        shaders[RendererShader::PBRP]->compile(shaderPath / "PBR.hlsl", RHIShaderType::Pixel);
        // clang-format on
    }

    void Renderer::createTextures()
    {
        // clang-format off
        textures[RendererTexture::Cornell] = std::make_shared<RHITexture>("/Users/w6rsty/Pictures/cornell.png");

        std::uint32_t white = 0xFFFF00FF;
        RHITextureMip mip;
        mip.bytes.resize(4);
        std::memcpy(mip.bytes.data(), &white, 4);
        RHITextureSlice slice;
        slice.mips.push_back(mip);
        std::vector<RHITextureSlice> data;
        data.push_back(slice);
        
        textures[RendererTexture::Placeholder] = std::make_shared<RHITexture>(RHITextureType::Texture2D, 1, 1, 1, 1, RHIFormat::R8G8B8A8Unorm, RHITextureViewUsageFlagBits::Srv | RHITextureViewUsageFlagBits::ClearOrBlit, data, "placeholder");

        // clang-format on
        for (std::size_t i = 0; i < textures.size(); ++i)
        {
            if (auto& texture = textures[i]; !texture || !texture->isValid())
            {
                WS_LOG_WARN(
                    "Renderer",
                    "Texture {} load failed",
                    renderTextureToString(static_cast<RendererTexture>(i)));
                // prevent usage of invalid texture
                texture = nullptr;
            }
        }
    }

    void Renderer::createSamplers()
    {
        // clang-format off
        samplers[RHISamplerType::CompareDepth]        = std::make_shared<RHISampler>(RHIFilter::Linear,  RHIFilter::Linear,  RHIFilter::Nearest, RHISamplerAddressMode::ClampToBorder, RHICompareOperation::Greater, true,  0.0f, 0.0f);
        samplers[RHISamplerType::PointClampEdge]      = std::make_shared<RHISampler>(RHIFilter::Nearest, RHIFilter::Nearest, RHIFilter::Nearest, RHISamplerAddressMode::ClampToEdge,   RHICompareOperation::Never,   false, 0.0f, 0.0f);
        samplers[RHISamplerType::PointClampBorder]    = std::make_shared<RHISampler>(RHIFilter::Nearest, RHIFilter::Nearest, RHIFilter::Nearest, RHISamplerAddressMode::ClampToBorder, RHICompareOperation::Never,   false, 0.0f, 0.0f);
        samplers[RHISamplerType::Wrap]                = std::make_shared<RHISampler>(RHIFilter::Nearest, RHIFilter::Nearest, RHIFilter::Nearest, RHISamplerAddressMode::Wrap,          RHICompareOperation::Never,   false, 0.0f, 0.0f);
        samplers[RHISamplerType::BilinearClampEdge]   = std::make_shared<RHISampler>(RHIFilter::Linear,  RHIFilter::Linear,  RHIFilter::Nearest, RHISamplerAddressMode::ClampToEdge,   RHICompareOperation::Never,   false, 0.0f, 0.0f);
        samplers[RHISamplerType::BilinearClampBorder] = std::make_shared<RHISampler>(RHIFilter::Linear,  RHIFilter::Linear,  RHIFilter::Nearest, RHISamplerAddressMode::ClampToBorder, RHICompareOperation::Never,   false, 0.0f, 0.0f);
        samplers[RHISamplerType::BilinearWrap]        = std::make_shared<RHISampler>(RHIFilter::Linear,  RHIFilter::Linear,  RHIFilter::Nearest, RHISamplerAddressMode::Wrap,          RHICompareOperation::Never,   false, 0.0f, 0.0f);
        samplers[RHISamplerType::TrilinearClamp]      = std::make_shared<RHISampler>(RHIFilter::Linear,  RHIFilter::Linear,  RHIFilter::Linear,  RHISamplerAddressMode::ClampToEdge,   RHICompareOperation::Never,   false, 0.0f, 0.0f);
        samplers[RHISamplerType::AnisotropicClamp]    = std::make_shared<RHISampler>(RHIFilter::Linear,  RHIFilter::Linear,  RHIFilter::Linear,  RHISamplerAddressMode::ClampToEdge,   RHICompareOperation::Never,   false, 0.0f, 0.0f);
        // clang-format on
    }

    void Renderer::createStandardMeshes()
    {
        std::vector<RHIVertexPosUvNrmTan> vertices;
        std::vector<std::uint32_t> indices;

        // clang-format off
        geometry::generateQuad3D(vertices, indices);
        standardMeshes[geometry::GeometryType::Quad3D] = std::make_shared<Mesh>();
        standardMeshes[geometry::GeometryType::Quad3D]->addGeometry(vertices,indices);
        standardMeshes[geometry::GeometryType::Quad3D]->createGPUBuffers();

        vertices.clear();
        indices.clear();

        geometry::generateCube(vertices, indices);
        standardMeshes[geometry::GeometryType::Cube] = std::make_shared<Mesh>();
        standardMeshes[geometry::GeometryType::Cube]->addGeometry(vertices, indices);
        standardMeshes[geometry::GeometryType::Cube]->createGPUBuffers();
        // clang-format on
    }

    void Renderer::destroyResources()
    {
        rasterizerStates.fill(nullptr);
        depthStencilStates.fill(nullptr);
        blendStates.fill(nullptr);
        renderTargets.fill(nullptr);
        shaders.fill(nullptr);
        textures.fill(nullptr);
        samplers.fill(nullptr);
        standardMeshes.fill(nullptr);
    }

    RHIRasterizerState*
    Renderer::getRasterizerState(RendererRasterizerState const state)
    {
        return rasterizerStates[state].get();
    }

    RHIDepthStencilState*
    Renderer::getDepthStencilState(RendererDepthStencilState const state)
    {
        return depthStencilStates[state].get();
    }

    RHIBlendState* Renderer::getBlendState(RendererBlendState const state)
    {
        return blendStates[state].get();
    }

    RHITexture* Renderer::getRenderTarget(RendererTarget const target)
    {
        return renderTargets[target].get();
    }

    RHIShader* Renderer::getShader(RendererShader const shader)
    {
        return shaders[shader].get();
    }

    RHITexture* Renderer::getTexture(RendererTexture const texture)
    {
        return textures[texture].get();
    }

    RHISampler* Renderer::getSampler(RHISamplerType const sampler)
    {
        return samplers[sampler].get();
    }

    Mesh* Renderer::getStandardMesh(geometry::GeometryType const type)
    {
        return standardMeshes[type].get();
    }

} // namespace worse
