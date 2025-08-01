#include "Types.hpp"
#include "Platform.hpp"
#include "RHIBuffer.hpp"
#include "RHIShader.hpp"
#include "RHIVertex.hpp"
#include "RHITexture.hpp"
#include "RHISampler.hpp"
#include "RHIDevice.hpp"
#include "Pipeline/RHIBlendState.hpp"
#include "Pipeline/RHIPipelineState.hpp"
#include "Pipeline/RHIRasterizerState.hpp"
#include "Pipeline/RHIDepthStencilState.hpp"
#include "Renderer.hpp"

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
        EnumArray<geometry::GeometryType, std::shared_ptr<Mesh>>                    standardMeshes;
        EnumArray<RendererPSO, RHIPipelineState>                                    pipelineStates;

        std::shared_ptr<RHIBuffer> materialBuffer;
        // clang-format on
    } // namespace

    void Renderer::createRasterizerStates()
    {
        // clang-format off
        rasterizerStates[RendererRasterizerState::Solid]     = std::make_shared<RHIRasterizerState>(RHIPolygonMode::Solid, RHICullMode::None, RHIFrontFace::CCW, 1.0f, false, 0.0f, 0.0f, 0.0f);
        rasterizerStates[RendererRasterizerState::Wireframe] = std::make_shared<RHIRasterizerState>(RHIPolygonMode::Wirefame, RHICullMode::None, RHIFrontFace::CCW, 1.0f, false, 0.0f, 0.0f, 0.0f);
        // clang-format on
    }

    void Renderer::createDepthStencilStates()
    {
        // clang-format off
        //                                                                                                test  | write  | compare
        depthStencilStates[RendererDepthStencilState::Off]              = std::make_shared<RHIDepthStencilState>(false,  false,   RHICompareOperation::Never);
        depthStencilStates[RendererDepthStencilState::ReadWrite]        = std::make_shared<RHIDepthStencilState>(true,   true,    RHICompareOperation::GreaterEqual);
        depthStencilStates[RendererDepthStencilState::ReadEqual]        = std::make_shared<RHIDepthStencilState>(true,   false,   RHICompareOperation::Equal);
        depthStencilStates[RendererDepthStencilState::ReadGreaterEqual] = std::make_shared<RHIDepthStencilState>(true,   false,   RHICompareOperation::GreaterEqual);
        depthStencilStates[RendererDepthStencilState::ReadLessEqual]    = std::make_shared<RHIDepthStencilState>(true,   false,   RHICompareOperation::LessEqual);
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
        u32 renderWidth  = static_cast<u32>(renderResolution.x);
        u32 renderHeight = static_cast<u32>(renderResolution.y);

        RHIFormat standardFormat = RHIFormat::B8R8G8A8Unorm;

        std::vector<RHITextureSlice> dummy;
        renderTargets[RendererTarget::Render] = std::make_shared<RHITexture>(RHITextureType::Texture2D, renderWidth, renderHeight, 1, 1, standardFormat, RHITextureViewFlagBits::RenderTargetView | RHITextureViewFlagBits::UnorderedAccessView | RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, dummy, "render");
        renderTargets[RendererTarget::Output] = std::make_shared<RHITexture>(RHITextureType::Texture2D, renderWidth, renderHeight, 1, 1, standardFormat, RHITextureViewFlagBits::RenderTargetView | RHITextureViewFlagBits::UnorderedAccessView | RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, dummy, "output");
        renderTargets[RendererTarget::Depth]  = std::make_shared<RHITexture>(RHITextureType::Texture2D, renderWidth, renderHeight, 1, 1, RHIFormat::D32Float, RHITextureViewFlagBits::DepthStencilView | RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, dummy, "depth");
        // clang-format on
    }

    void Renderer::createShaders()
    {
        std::filesystem::path shaderDir = std::filesystem::path{worse::EngineDirectory} / "Shaders";
        WS_LOG_INFO("Renderer", "Shader directory: {}", shaderDir.string());

        // clang-format off
        shaders[RendererShader::PlaceholderV] = std::make_shared<RHIShader>("PlaceholderV");
        shaders[RendererShader::PlaceholderV]->compile(shaderDir / "Placeholder.hlsl", RHIShaderType::Vertex, RHIVertexType::PosUvNrmTan);
        shaders[RendererShader::PlaceholderP] = std::make_shared<RHIShader>("PlaceholderP");
        shaders[RendererShader::PlaceholderP]->compile(shaderDir / "Placeholder.hlsl", RHIShaderType::Pixel);

        shaders[RendererShader::DepthPrepassV] = std::make_shared<RHIShader>("DepthPrepassV");
        shaders[RendererShader::DepthPrepassV]->compile(shaderDir / "DepthPrepass.hlsl", RHIShaderType::Vertex, RHIVertexType::PosUvNrmTan);
        shaders[RendererShader::DepthPrepassP] = std::make_shared<RHIShader>("DepthPrepassP");
        shaders[RendererShader::DepthPrepassP]->compile(shaderDir / "DepthPrepass.hlsl", RHIShaderType::Pixel);

        shaders[RendererShader::KuwaharaC] = std::make_shared<RHIShader>("KuwaharaC");
        shaders[RendererShader::KuwaharaC]->compile(shaderDir / "Kuwahara.hlsl", RHIShaderType::Compute);

        shaders[RendererShader::LineV] = std::make_shared<RHIShader>("LineV");
        shaders[RendererShader::LineV]->compile(shaderDir / "Line.hlsl", RHIShaderType::Vertex, RHIVertexType::PosUvNrmTan);
        shaders[RendererShader::LineP] = std::make_shared<RHIShader>("LineP");
        shaders[RendererShader::LineP]->compile(shaderDir / "Line.hlsl", RHIShaderType::Pixel);

        shaders[RendererShader::PointV] = std::make_shared<RHIShader>("PointV");
        shaders[RendererShader::PointV]->compile(shaderDir / "Point.hlsl", RHIShaderType::Vertex, RHIVertexType::PosUvNrmTan);
        shaders[RendererShader::PointP] = std::make_shared<RHIShader>("PointP");
        shaders[RendererShader::PointP]->compile(shaderDir / "Point.hlsl", RHIShaderType::Pixel);

        shaders[RendererShader::PBRV] = std::make_shared<RHIShader>("PBRV");
        shaders[RendererShader::PBRV]->compile(shaderDir / "PBR.hlsl", RHIShaderType::Vertex, RHIVertexType::PosUvNrmTan);
        shaders[RendererShader::PBRP] = std::make_shared<RHIShader>("PBRP");
        shaders[RendererShader::PBRP]->compile(shaderDir / "PBR.hlsl", RHIShaderType::Pixel);

        shaders[RendererShader::DistortionV] = std::make_shared<RHIShader>("DistortionV");
        shaders[RendererShader::DistortionV]->compile(shaderDir / "Distortion.hlsl", RHIShaderType::Vertex, RHIVertexType::PosUvNrmTan);
        shaders[RendererShader::DistortionP] = std::make_shared<RHIShader>("DistortionP");
        shaders[RendererShader::DistortionP]->compile(shaderDir / "Distortion.hlsl", RHIShaderType::Pixel);
        // clang-format on
    }

    void Renderer::createTextures()
    {
        // clang-format off

        {
            u32 const width = 4;
            u32 const height = 4;
            u32 const white = 0xFFFFFFFF;
            u32 const grey = 0xFF808080;
            u32 pixels[static_cast<usize>(width * height)];
            for (usize i = 0; i < static_cast<usize>(width * height); ++i)
            {
                usize x = i % width;
                usize y = i / width;
                pixels[i] = ((x + y) % 2 == 0) ? white : grey;
            }
            RHITextureMip mip;
            mip.bytes.resize(sizeof(u32) * static_cast<usize>(width * height));
            std::memcpy(mip.bytes.data(), pixels, sizeof(u32) * static_cast<usize>(width * height));
            RHITextureSlice slice;
            slice.mips.push_back(mip);
            std::vector<RHITextureSlice> data;
            data.push_back(slice);
        
            textures[RendererTexture::Placeholder] = std::make_shared<RHITexture>(RHITextureType::Texture2D, width, height, 1, 1, RHIFormat::R8G8B8A8Unorm, RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, data, "Placeholder");
        }
        // Default Albedo Map: (1.0, 1.0, 1.0, 1.0) = (255, 255, 255, 255) in RGBA8
        {
            u32 albedo = 0xFFFFFFFF; // ABGR format: A=255, B=255, G=255, R=255
            RHITextureMip mip;
            mip.bytes.resize(4);
            std::memcpy(mip.bytes.data(), &albedo, 4);
            RHITextureSlice slice;
            slice.mips.push_back(mip);
            std::vector<RHITextureSlice> data;
            data.push_back(slice);
        
            textures[RendererTexture::DefaultAlbedo] = std::make_shared<RHITexture>(RHITextureType::Texture2D, 1, 1, 1, 1, RHIFormat::R8G8B8A8Unorm, RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, data, "DefaultAlbedo");
        }
        // Default Normal Map: (0.5, 0.5, 1.0, 1.0) in tangent space = (128, 128, 255, 255) in RGBA8
        {
            u32 normal = 0xFFFF8080; // ABGR format: A=255, B=128, G=128, R=255
            RHITextureMip mip;
            mip.bytes.resize(4);
            std::memcpy(mip.bytes.data(), &normal, 4);
            RHITextureSlice slice;
            slice.mips.push_back(mip);
            std::vector<RHITextureSlice> data;
            data.push_back(slice);
        
            textures[RendererTexture::DefaultNormal] = std::make_shared<RHITexture>(RHITextureType::Texture2D, 1, 1, 1, 1, RHIFormat::R8G8B8A8Unorm, RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, data, "DefaultNormal");
        }
        // Default Metallic: 0.0 (non-metallic) = (0, 0, 0, 255) in RGBA8
        {
            u32 metallic = 0xFF000000; // ABGR format: A=255, B=0, G=0, R=0
            RHITextureMip mip;
            mip.bytes.resize(4);
            std::memcpy(mip.bytes.data(), &metallic, 4);
            RHITextureSlice slice;
            slice.mips.push_back(mip);
            std::vector<RHITextureSlice> data;
            data.push_back(slice);
        
            textures[RendererTexture::DefaultMetallic] = std::make_shared<RHITexture>(RHITextureType::Texture2D, 1, 1, 1, 1, RHIFormat::R8G8B8A8Unorm, RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, data, "DefaultMetallic");
        }
        // Default Roughness: 0.5 (medium roughness) = (128, 128, 128, 255) in RGBA8
        {
            u32 roughness = 0xFF808080; // ABGR format: A=255, B=128, G=128, R=128
            RHITextureMip mip;
            mip.bytes.resize(4);
            std::memcpy(mip.bytes.data(), &roughness, 4);
            RHITextureSlice slice;
            slice.mips.push_back(mip);
            std::vector<RHITextureSlice> data;
            data.push_back(slice);
        
            textures[RendererTexture::DefaultRoughness] = std::make_shared<RHITexture>(RHITextureType::Texture2D, 1, 1, 1, 1, RHIFormat::R8G8B8A8Unorm, RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, data, "DefaultRoughness");
        }
        // Default Ambient Occlusion: 1.0 (no occlusion) = (255, 255, 255, 255) in RGBA8
        {
            u32 ambientOcclusion = 0xFFFFFFFF; // ABGR format: A=255, B=255, G=255, R=255
            RHITextureMip mip;
            mip.bytes.resize(4);
            std::memcpy(mip.bytes.data(), &ambientOcclusion, 4);
            RHITextureSlice slice;
            slice.mips.push_back(mip);
            std::vector<RHITextureSlice> data;
            data.push_back(slice);
        
            textures[RendererTexture::DefaultAmbientOcclusion] = std::make_shared<RHITexture>(RHITextureType::Texture2D, 1, 1, 1, 1, RHIFormat::R8G8B8A8Unorm, RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, data, "DefaultAmbientOcclusion");
        }
        // Default Emissive: (0.0, 0.0, 0.0) (no emission) = (0, 0, 0, 255) in RGBA8
        {
            u32 emissive = 0xFF000000; // ABGR format: A=255, B=0, G=0, R=0
            RHITextureMip mip;
            mip.bytes.resize(4);
            std::memcpy(mip.bytes.data(), &emissive, 4);
            RHITextureSlice slice;
            slice.mips.push_back(mip);
            std::vector<RHITextureSlice> data;
            data.push_back(slice);
        
            textures[RendererTexture::DefaultEmissive] = std::make_shared<RHITexture>(RHITextureType::Texture2D, 1, 1, 1, 1, RHIFormat::R8G8B8A8Unorm, RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, data, "DefaultEmissive");
        }

        // clang-format on
        for (usize i = 0; i < textures.size(); ++i)
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
        std::vector<u32> indices;

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

        vertices.clear();
        indices.clear();

        geometry::generateSphere(vertices, indices);
        standardMeshes[geometry::GeometryType::Sphere] = std::make_shared<Mesh>();
        standardMeshes[geometry::GeometryType::Sphere]->addGeometry(vertices, indices);
        standardMeshes[geometry::GeometryType::Sphere]->createGPUBuffers();

        vertices.clear();
        indices.clear();

        geometry::generateCylinder(vertices, indices);
        standardMeshes[geometry::GeometryType::Cylinder] = std::make_shared<Mesh>();
        standardMeshes[geometry::GeometryType::Cylinder]->addGeometry(vertices, indices);
        standardMeshes[geometry::GeometryType::Cylinder]->createGPUBuffers();

        vertices.clear();
        indices.clear();

        geometry::generateCapsule(vertices, indices);
        standardMeshes[geometry::GeometryType::Capsule] = std::make_shared<Mesh>();
        standardMeshes[geometry::GeometryType::Capsule]->addGeometry(vertices, indices);
        standardMeshes[geometry::GeometryType::Capsule]->createGPUBuffers();
        // clang-format on
    }

    void Renderer::createPipelineStates()
    {
        // clang-format off
        pipelineStates[RendererPSO::DepthPrepass] =
            RHIPipelineStateBuilder()
                .setName("DepthPrepass")
                .setType(RHIPipelineType::Graphics)
                .setPrimitiveTopology(RHIPrimitiveTopology::TriangleList)
                .setRasterizerState(Renderer::getRasterizerState(RendererRasterizerState::Solid))
                .setDepthStencilState(Renderer::getDepthStencilState(RendererDepthStencilState::ReadWrite))
                .setBlendState(Renderer::getBlendState(RendererBlendState::Off))
                .addShader(Renderer::getShader(RendererShader::DepthPrepassV))
                .addShader(Renderer::getShader(RendererShader::DepthPrepassP))
                .setRenderTargetDepthTexture(Renderer::getRenderTarget(RendererTarget::Depth))
                .setScissor({0, 0, 1200, 720})
                .setViewport(Renderer::getViewport())
                .setClearDepth(0.0f) // clear with far value
                .build();
        RHIDevice::getPipeline(pipelineStates[RendererPSO::DepthPrepass]);

        pipelineStates[RendererPSO::PBR] =
            RHIPipelineStateBuilder()
                .setName("PBRPass")
                .setType(RHIPipelineType::Graphics)
                .setPrimitiveTopology(RHIPrimitiveTopology::TriangleList)
                .setRasterizerState(Renderer::getRasterizerState(RendererRasterizerState::Solid))
                .setDepthStencilState(Renderer::getDepthStencilState(RendererDepthStencilState::ReadGreaterEqual))
                .setBlendState(Renderer::getBlendState(RendererBlendState::Off))
                .addShader(Renderer::getShader(RendererShader::PBRV))
                .addShader(Renderer::getShader(RendererShader::PBRP))
                .setRenderTargetColorTexture(0, Renderer::getRenderTarget(RendererTarget::Render))
                .setRenderTargetDepthTexture(Renderer::getRenderTarget(RendererTarget::Depth))
                .setScissor({0, 0, 1200, 720})
                .setViewport(Renderer::getViewport())
                .setClearColor(Color{0.2f, 0.2f, 0.2f, 1.0f})
                .setClearDepth(2.0f)
                .build();
        RHIDevice::getPipeline(pipelineStates[RendererPSO::PBR]);

        pipelineStates[RendererPSO::Wireframe] =
            RHIPipelineStateBuilder()
                .setName("WireFramePass")
                .setType(RHIPipelineType::Graphics)
                .setPrimitiveTopology(RHIPrimitiveTopology::TriangleList)
                .setRasterizerState(Renderer::getRasterizerState(RendererRasterizerState::Wireframe))
                .setDepthStencilState(Renderer::getDepthStencilState(RendererDepthStencilState::Off))
                .setBlendState(Renderer::getBlendState(RendererBlendState::Off))
                .addShader(Renderer::getShader(RendererShader::LineV))
                .addShader(Renderer::getShader(RendererShader::LineP))
                .setRenderTargetColorTexture(0, Renderer::getRenderTarget(RendererTarget::Render))
                .setScissor({0, 0, 1200, 720})
                .setViewport(Renderer::getViewport())
                .build();
        RHIDevice::getPipeline(pipelineStates[RendererPSO::Wireframe]);

        pipelineStates[RendererPSO::Point] =
            RHIPipelineStateBuilder()
                .setName("PointPass")
                .setType(RHIPipelineType::Graphics)
                .setPrimitiveTopology(RHIPrimitiveTopology::PointList)
                .setRasterizerState(Renderer::getRasterizerState(RendererRasterizerState::Solid))
                .setDepthStencilState(Renderer::getDepthStencilState(RendererDepthStencilState::ReadGreaterEqual))
                .setBlendState(Renderer::getBlendState(RendererBlendState::Off))
                .addShader(Renderer::getShader(RendererShader::PointV))
                .addShader(Renderer::getShader(RendererShader::PointP))
                .setRenderTargetColorTexture(0, Renderer::getRenderTarget(RendererTarget::Render))
                .setRenderTargetDepthTexture(Renderer::getRenderTarget(RendererTarget::Depth))
                .setScissor({0, 0, 1200, 720})
                .setViewport(Renderer::getViewport())
                .setClearDepth(2.0f)
                .build();
        RHIDevice::getPipeline(pipelineStates[RendererPSO::Point]);

        pipelineStates[RendererPSO::PostProcessing] =
            RHIPipelineStateBuilder()
                .setName("PostProcessingPass")
                .setType(RHIPipelineType::Compute)
                .addShader(Renderer::getShader(RendererShader::KuwaharaC))
                .build();
        RHIDevice::getPipeline(pipelineStates[RendererPSO::PostProcessing]);
        // clang-format on
    }

    void
    Renderer::createMaterialBuffers(std::span<StandardMaterialGPU> materials)
    {
        materialBuffer =
            std::make_shared<RHIBuffer>(RHIBufferUsageFlagBits::Storage,
                                        sizeof(StandardMaterialGPU),
                                        materials.size(),
                                        materials.data(),
                                        false,
                                        "MaterialBuffer");
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
        // pipelineStates
        materialBuffer.reset();
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

    std::shared_ptr<Mesh> Renderer::getStandardMesh(geometry::GeometryType const type)
    {
        return standardMeshes[type];
    }

    RHIPipelineState const& Renderer::getPipelineState(RendererPSO const pso)
    {
        return pipelineStates[pso];
    }

    RHIBuffer* Renderer::getMaterialBuffer()
    {
        return materialBuffer.get();
    }

} // namespace worse
