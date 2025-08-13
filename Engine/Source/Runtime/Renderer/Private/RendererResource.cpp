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
        EnumArray<RendererRasterizerState, std::unique_ptr<RHIRasterizerState>> rasterizerStates;
        EnumArray<RendererDepthStencilState, std::unique_ptr<RHIDepthStencilState>> depthStencilStates;
        EnumArray<RendererBlendState, std::unique_ptr<RHIBlendState>> blendStates;
        EnumArray<RendererTarget, std::unique_ptr<RHITexture>> renderTargets;
        EnumArray<RendererShader, std::unique_ptr<RHIShader>> shaders;
        EnumArray<RendererTexture, std::unique_ptr<RHITexture>> textures;
        EnumArray<RHISamplerType, std::unique_ptr<RHISampler>> samplers;
        EnumArray<geometry::GeometryType, std::unique_ptr<Mesh>> standardMeshes;
        EnumArray<RendererPSOType, RHIPipelineState> pipelineStates;

        std::shared_ptr<RHIBuffer> materialBuffer;
    } // namespace

    void Renderer::createRasterizerStates()
    {
        rasterizerStates[RendererRasterizerState::DepthPrepass] = std::make_unique<RHIRasterizerState>(RHIPolygonMode::Solid, RHICullMode::Back);
        rasterizerStates[RendererRasterizerState::Solid]        = std::make_unique<RHIRasterizerState>(RHIPolygonMode::Solid, RHICullMode::Back, RHIFrontFace::CCW, 1e-6f);
        rasterizerStates[RendererRasterizerState::Wireframe]    = std::make_unique<RHIRasterizerState>(RHIPolygonMode::Wirefame, RHICullMode::None);
    }

    void Renderer::createDepthStencilStates()
    {
        depthStencilStates[RendererDepthStencilState::Off]              = std::make_unique<RHIDepthStencilState>(false, false, RHICompareOperation::Never);
        depthStencilStates[RendererDepthStencilState::ReadWrite]        = std::make_unique<RHIDepthStencilState>(true, true, RHICompareOperation::GreaterEqual);
        depthStencilStates[RendererDepthStencilState::ReadEqual]        = std::make_unique<RHIDepthStencilState>(true, false, RHICompareOperation::Equal);
        depthStencilStates[RendererDepthStencilState::ReadGreaterEqual] = std::make_unique<RHIDepthStencilState>(true, false, RHICompareOperation::GreaterEqual);
        depthStencilStates[RendererDepthStencilState::ReadLessEqual]    = std::make_unique<RHIDepthStencilState>(true, false, RHICompareOperation::LessEqual);
    }

    void Renderer::createBlendStates()
    {
        blendStates[RendererBlendState::Off] = std::make_unique<RHIBlendState>(false, RHIBlendFactor::SrcAlpha, RHIBlendFactor::OneMinusSrcAlpha, RHIBlendOperation::Add, RHIBlendFactor::One, RHIBlendFactor::One, RHIBlendOperation::Add, 1.0f);
    }

    void Renderer::createRendererTarget()
    {
        math::Vector2 renderResolution = getResolutionRender();
        u32 renderWidth                = static_cast<u32>(renderResolution.x);
        u32 renderHeight               = static_cast<u32>(renderResolution.y);

        RHIFormat standardFormat = RHIFormat::B8R8G8A8Unorm;

        std::vector<RHITextureSlice> dummy;
        renderTargets[RendererTarget::Render]        = std::make_unique<RHITexture>(RHITextureType::Texture2D, renderWidth, renderHeight, 1, 1, standardFormat, RHITextureViewFlagBits::RenderTargetView | RHITextureViewFlagBits::UnorderedAccessView | RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, dummy, "render");
        renderTargets[RendererTarget::Output]        = std::make_unique<RHITexture>(RHITextureType::Texture2D, renderWidth, renderHeight, 1, 1, standardFormat, RHITextureViewFlagBits::RenderTargetView | RHITextureViewFlagBits::UnorderedAccessView | RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, dummy, "output");
        renderTargets[RendererTarget::GBufferNormal] = std::make_unique<RHITexture>(RHITextureType::Texture2D, renderWidth, renderHeight, 1, 1, standardFormat, RHITextureViewFlagBits::RenderTargetView | RHITextureViewFlagBits::UnorderedAccessView | RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, dummy, "gbuffer_normal");
        renderTargets[RendererTarget::GBufferAlbedo] = std::make_unique<RHITexture>(RHITextureType::Texture2D, renderWidth, renderHeight, 1, 1, standardFormat, RHITextureViewFlagBits::RenderTargetView | RHITextureViewFlagBits::UnorderedAccessView | RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, dummy, "gbuffer_albedo");
        renderTargets[RendererTarget::Depth]         = std::make_unique<RHITexture>(RHITextureType::Texture2D, renderWidth, renderHeight, 1, 1, RHIFormat::D32Float, RHITextureViewFlagBits::DepthStencilView | RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, dummy, "depth");
    }

    void Renderer::createShaders()
    {
        std::filesystem::path shaderDir = std::filesystem::path{worse::EngineDirectory} / "Shaders";
        WS_LOG_INFO("Renderer", "Shader directory: {}", shaderDir.string());

#define MAKE_SHADER_VP(shaderName, vertexType)                                                                           \
    shaders[RendererShader::shaderName##V] = std::make_unique<RHIShader>(#shaderName "V");                               \
    shaders[RendererShader::shaderName##V]->compile(shaderDir / #shaderName ".hlsl", RHIShaderType::Vertex, vertexType); \
    shaders[RendererShader::shaderName##P] = std::make_unique<RHIShader>(#shaderName "P");                               \
    shaders[RendererShader::shaderName##P]->compile(shaderDir / #shaderName ".hlsl", RHIShaderType::Pixel);

        MAKE_SHADER_VP(Placeholder, RHIVertexType::PosUvNrmTan);
        MAKE_SHADER_VP(DepthPrepass, RHIVertexType::PosUvNrmTan);
        MAKE_SHADER_VP(Line, RHIVertexType::PosUvNrmTan);
        MAKE_SHADER_VP(Point, RHIVertexType::PosUvNrmTan);
        MAKE_SHADER_VP(PBR, RHIVertexType::PosUvNrmTan);

#undef MAKE_SHADER_VP

#define MAKE_SHADER_C(shaderName)                                                          \
    shaders[RendererShader::shaderName##C] = std::make_unique<RHIShader>(#shaderName "C"); \
    shaders[RendererShader::shaderName##C]->compile(shaderDir / #shaderName ".hlsl", RHIShaderType::Compute);

        MAKE_SHADER_C(PostFX);

#undef MAKE_SHADER_C
    }

    void Renderer::createTextures()
    {
        {
            u32 const width  = 4;
            u32 const height = 4;
            u32 const white  = 0xFFFFFFFF;
            u32 const grey   = 0xFF808080;
            u32 pixels[static_cast<usize>(width * height)];
            for (usize i = 0; i < static_cast<usize>(width * height); ++i)
            {
                usize x   = i % width;
                usize y   = i / width;
                pixels[i] = ((x + y) % 2 == 0) ? white : grey;
            }
            RHITextureMip mip;
            mip.bytes.resize(sizeof(u32) * static_cast<usize>(width * height));
            std::memcpy(mip.bytes.data(), pixels, sizeof(u32) * static_cast<usize>(width * height));
            RHITextureSlice slice;
            slice.mips.push_back(mip);
            std::vector<RHITextureSlice> data;
            data.push_back(slice);

            textures[RendererTexture::Placeholder] = std::make_unique<RHITexture>(RHITextureType::Texture2D, width, height, 1, 1, RHIFormat::R8G8B8A8Unorm, RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, data, "Placeholder");
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

            textures[RendererTexture::DefaultAlbedo] = std::make_unique<RHITexture>(RHITextureType::Texture2D, 1, 1, 1, 1, RHIFormat::R8G8B8A8Unorm, RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, data, "DefaultAlbedo");
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

            textures[RendererTexture::DefaultNormal] = std::make_unique<RHITexture>(RHITextureType::Texture2D, 1, 1, 1, 1, RHIFormat::R8G8B8A8Unorm, RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, data, "DefaultNormal");
        }
        // Default Metallic Roughness: 1.0
        {
            u32 metallic = 0xFF000000; // ABGR format: A=255, B=0, G=0, R=0
            RHITextureMip mip;
            mip.bytes.resize(4);
            std::memcpy(mip.bytes.data(), &metallic, 4);
            RHITextureSlice slice;
            slice.mips.push_back(mip);
            std::vector<RHITextureSlice> data;
            data.push_back(slice);

            textures[RendererTexture::DefaultMetallicRoughness] = std::make_unique<RHITexture>(RHITextureType::Texture2D, 1, 1, 1, 1, RHIFormat::R8G8B8A8Unorm, RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, data, "DefaultMetallic");
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

            textures[RendererTexture::DefaultAmbientOcclusion] = std::make_unique<RHITexture>(RHITextureType::Texture2D, 1, 1, 1, 1, RHIFormat::R8G8B8A8Unorm, RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, data, "DefaultAmbientOcclusion");
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

            textures[RendererTexture::DefaultEmissive] = std::make_unique<RHITexture>(RHITextureType::Texture2D, 1, 1, 1, 1, RHIFormat::R8G8B8A8Unorm, RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, data, "DefaultEmissive");
        }

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
        samplers[RHISamplerType::CompareDepth] = std::make_unique<RHISampler>(RHIFilter::Linear, RHIFilter::Linear, RHIFilter::Nearest, RHISamplerAddressMode::ClampToBorder, RHICompareOperation::Greater, true, 0.0f, 0.0f);

        // 邻近
        samplers[RHISamplerType::PointClampEdge]   = std::make_unique<RHISampler>(RHIFilter::Nearest, RHIFilter::Nearest, RHIFilter::Nearest, RHISamplerAddressMode::ClampToEdge);
        samplers[RHISamplerType::PointClampBorder] = std::make_unique<RHISampler>(RHIFilter::Nearest, RHIFilter::Nearest, RHIFilter::Nearest, RHISamplerAddressMode::ClampToBorder);
        samplers[RHISamplerType::PointWrap]        = std::make_unique<RHISampler>(RHIFilter::Nearest, RHIFilter::Nearest, RHIFilter::Nearest, RHISamplerAddressMode::Wrap);
        // 双线性
        samplers[RHISamplerType::BilinearClampEdge]   = std::make_unique<RHISampler>(RHIFilter::Linear, RHIFilter::Linear, RHIFilter::Nearest, RHISamplerAddressMode::ClampToEdge);
        samplers[RHISamplerType::BilinearClampBorder] = std::make_unique<RHISampler>(RHIFilter::Linear, RHIFilter::Linear, RHIFilter::Nearest, RHISamplerAddressMode::ClampToBorder);
        samplers[RHISamplerType::BilinearWrap]        = std::make_unique<RHISampler>(RHIFilter::Linear, RHIFilter::Linear, RHIFilter::Nearest, RHISamplerAddressMode::Wrap);
        // 三线性
        samplers[RHISamplerType::TrilinearClamp]   = std::make_unique<RHISampler>(RHIFilter::Linear, RHIFilter::Linear, RHIFilter::Linear, RHISamplerAddressMode::ClampToEdge);
        samplers[RHISamplerType::AnisotropicClamp] = std::make_unique<RHISampler>(RHIFilter::Linear, RHIFilter::Linear, RHIFilter::Linear, RHISamplerAddressMode::ClampToEdge);
    }

    void Renderer::createStandardMeshes()
    {
        std::vector<RHIVertexPosUvNrmTan> vertices;
        std::vector<u32> indices;

        // clang-format off
        geometry::generateQuad3D(vertices, indices);
        standardMeshes[geometry::GeometryType::Quad3D] = std::make_unique<Mesh>();
        standardMeshes[geometry::GeometryType::Quad3D]->addGeometry(vertices,indices);
        standardMeshes[geometry::GeometryType::Quad3D]->createGPUBuffers();

        vertices.clear();
        indices.clear();

        geometry::generateCube(vertices, indices);
        standardMeshes[geometry::GeometryType::Cube] = std::make_unique<Mesh>();
        standardMeshes[geometry::GeometryType::Cube]->addGeometry(vertices, indices);
        standardMeshes[geometry::GeometryType::Cube]->createGPUBuffers();

        vertices.clear();
        indices.clear();

        geometry::generateSphere(vertices, indices);
        standardMeshes[geometry::GeometryType::Sphere] = std::make_unique<Mesh>();
        standardMeshes[geometry::GeometryType::Sphere]->addGeometry(vertices, indices);
        standardMeshes[geometry::GeometryType::Sphere]->createGPUBuffers();

        vertices.clear();
        indices.clear();

        geometry::generateCylinder(vertices, indices);
        standardMeshes[geometry::GeometryType::Cylinder] = std::make_unique<Mesh>();
        standardMeshes[geometry::GeometryType::Cylinder]->addGeometry(vertices, indices);
        standardMeshes[geometry::GeometryType::Cylinder]->createGPUBuffers();

        vertices.clear();
        indices.clear();

        geometry::generateCapsule(vertices, indices);
        standardMeshes[geometry::GeometryType::Capsule] = std::make_unique<Mesh>();
        standardMeshes[geometry::GeometryType::Capsule]->addGeometry(vertices, indices);
        standardMeshes[geometry::GeometryType::Capsule]->createGPUBuffers();
        // clang-format on
    }

    void Renderer::createPipelineStates()
    {
        // clang-format off
        pipelineStates[RendererPSOType::DepthPrepass] =
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
                .build();
        RHIDevice::getPipeline(pipelineStates[RendererPSOType::DepthPrepass]);

        pipelineStates[RendererPSOType::PBR] =
            RHIPipelineStateBuilder()
                .setName("PBR")
                .setType(RHIPipelineType::Graphics)
                .setPrimitiveTopology(RHIPrimitiveTopology::TriangleList)
                .setRasterizerState(Renderer::getRasterizerState(RendererRasterizerState::Solid))
                .setDepthStencilState(Renderer::getDepthStencilState(RendererDepthStencilState::ReadGreaterEqual))
                .setBlendState(Renderer::getBlendState(RendererBlendState::Off))
                .addShader(Renderer::getShader(RendererShader::PBRV))
                .addShader(Renderer::getShader(RendererShader::PBRP))
                .setRenderTargetColorTexture(0, Renderer::getRenderTarget(RendererTarget::Render))
                .setRenderTargetColorTexture(1, Renderer::getRenderTarget(RendererTarget::GBufferNormal))
                .setRenderTargetColorTexture(2, Renderer::getRenderTarget(RendererTarget::GBufferAlbedo))
                .setRenderTargetDepthTexture(Renderer::getRenderTarget(RendererTarget::Depth))
                .setScissor({0, 0, 1200, 720})
                .setViewport(Renderer::getViewport())
                .setClearColor(Color{0.02f, 0.02f, 0.02f, 1.0f})
                .setClearDepth(2.0f)
                .build();
        RHIDevice::getPipeline(pipelineStates[RendererPSOType::PBR]);

        pipelineStates[RendererPSOType::Wireframe] =
            RHIPipelineStateBuilder()
                .setName("WireFrame")
                .setType(RHIPipelineType::Graphics)
                .setPrimitiveTopology(RHIPrimitiveTopology::TriangleList)
                .setRasterizerState(Renderer::getRasterizerState(RendererRasterizerState::Wireframe))
                .setDepthStencilState(Renderer::getDepthStencilState(RendererDepthStencilState::Off))
                .setBlendState(Renderer::getBlendState(RendererBlendState::Off))
                .addShader(Renderer::getShader(RendererShader::LineV))
                .addShader(Renderer::getShader(RendererShader::LineP))
                .setRenderTargetColorTexture(0, Renderer::getRenderTarget(RendererTarget::Output)) // 渲染到后处理之后
                .setScissor({0, 0, 1200, 720})
                .setViewport(Renderer::getViewport())
                .build();
        RHIDevice::getPipeline(pipelineStates[RendererPSOType::Wireframe]);

        pipelineStates[RendererPSOType::Point] =
            RHIPipelineStateBuilder()
                .setName("Point")
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
        RHIDevice::getPipeline(pipelineStates[RendererPSOType::Point]);

        pipelineStates[RendererPSOType::PostFX] =
            RHIPipelineStateBuilder()
                .setName("PostFX")
                .setType(RHIPipelineType::Compute)
                .addShader(Renderer::getShader(RendererShader::PostFXC))
                .build();
        RHIDevice::getPipeline(pipelineStates[RendererPSOType::PostFX]);
        // clang-format on
    }

    void Renderer::createMaterialBuffers(std::span<StandardMaterialGPU> materials)
    {
        materialBuffer = std::make_shared<RHIBuffer>(RHIBufferUsageFlagBits::Storage,
                                                     sizeof(StandardMaterialGPU),
                                                     materials.size(),
                                                     materials.data(),
                                                     false,
                                                     "MaterialBuffer");
    }

    void Renderer::destroyResources()
    {
        for (std::unique_ptr<worse::RHIRasterizerState>& rasterizerState : rasterizerStates)
        {
            rasterizerState.reset();
        }
        for (std::unique_ptr<worse::RHIDepthStencilState>& depthStencilState : depthStencilStates)
        {
            depthStencilState.reset();
        }

        for (std::unique_ptr<worse::RHIBlendState>& blendState : blendStates)
        {
            blendState.reset();
        }

        for (std::unique_ptr<worse::RHITexture>& renderTarget : renderTargets)
        {
            renderTarget.reset();
        }

        for (std::unique_ptr<worse::RHIShader>& shader : shaders)
        {
            shader.reset();
        }

        for (std::unique_ptr<RHITexture>& texture : textures)
        {
            texture.reset();
        }

        for (std::unique_ptr<RHISampler>& sampler : samplers)
        {
            sampler.reset();
        }

        for (std::unique_ptr<Mesh>& mesh : standardMeshes)
        {
            mesh.reset();
        }

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

    Mesh* Renderer::getStandardMesh(geometry::GeometryType const type)
    {
        return standardMeshes[type].get();
    }

    RHIPipelineState const& Renderer::getPipelineState(RendererPSOType const pso)
    {
        return pipelineStates[pso];
    }

    RHIBuffer* Renderer::getMaterialBuffer()
    {
        return materialBuffer.get();
    }

} // namespace worse
