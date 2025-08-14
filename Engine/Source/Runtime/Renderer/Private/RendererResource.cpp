#include "Types.hpp"
#include "Platform.hpp"
#include "RHIBuffer.hpp"
#include "RHIShader.hpp"
#include "RHIVertex.hpp"
#include "RHITexture.hpp"
#include "RHISampler.hpp"
#include "Pipeline/RHIBlendState.hpp"
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
        math::Vector2 resolution = getResolutionRender();
        u32 width                = static_cast<u32>(resolution.x);
        u32 height               = static_cast<u32>(resolution.y);

        std::vector<RHITextureSlice> dummy;

        renderTargets[RendererTarget::SceneHDR]  = std::make_unique<RHITexture>(RHITextureType::Texture2D, width, height, 1, 1, RHIFormat::R16G16B16A16Float, RHITextureViewFlagBits::RenderTargetView | RHITextureViewFlagBits::UnorderedAccessView | RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, dummy, "render");
        renderTargets[RendererTarget::ScreenHDR] = std::make_unique<RHITexture>(RHITextureType::Texture2D, width, height, 1, 1, RHIFormat::R16G16B16A16Float, RHITextureViewFlagBits::RenderTargetView | RHITextureViewFlagBits::UnorderedAccessView | RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, dummy, "output");

        // GBuffer
        renderTargets[RendererTarget::GBufferNormal] = std::make_unique<RHITexture>(RHITextureType::Texture2D, width, height, 1, 1, RHIFormat::B8R8G8A8Unorm, RHITextureViewFlagBits::RenderTargetView | RHITextureViewFlagBits::UnorderedAccessView | RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, dummy, "gbuffer_normal");
        renderTargets[RendererTarget::GBufferAlbedo] = std::make_unique<RHITexture>(RHITextureType::Texture2D, width, height, 1, 1, RHIFormat::B8R8G8A8Unorm, RHITextureViewFlagBits::RenderTargetView | RHITextureViewFlagBits::UnorderedAccessView | RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, dummy, "gbuffer_albedo");

        renderTargets[RendererTarget::Depth] = std::make_unique<RHITexture>(RHITextureType::Texture2D, width, height, 1, 1, RHIFormat::D32Float, RHITextureViewFlagBits::DepthStencilView | RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit, dummy, "depth");
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

    RHIBuffer* Renderer::getMaterialBuffer()
    {
        return materialBuffer.get();
    }

} // namespace worse
