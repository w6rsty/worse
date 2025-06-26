#pragma once
#include "Mesh.hpp"
#include "Geometry/GeometryGeneration.hpp"
#include "Math/Vector.hpp"
#include "RHIDefinitions.hpp"
#include "RendererDefinitions.hpp"

#include <cstdint>

namespace worse
{

    class Renderer
    {
    public:
        static void initialize();
        static void shutdown();
        static void tick();

        // swapchain
        static RHISwapchain* getSwapchain();
        static void blitToBackBuffer(RHICommandList* cmdList);
        // submit command list and present image to swapchain
        static void submitAndPresent();
        static void updateBuffers(RHICommandList* cmdList);

        static void setViewport(float const width, float const height);
        static RHIViewport const& getViewport();

        static RHIRasterizerState*
        getRasterizerState(RendererRasterizerState const state);
        static RHIDepthStencilState*
        getDepthStencilState(RendererDepthStencilState const state);
        static RHIBlendState* getBlendState(RendererBlendState const state);
        static RHITexture* getRenderTarget(RendererTarget const target);
        static RHIShader* getShader(RendererShader const shader);
        static RHITexture* getTexture(RendererTexture const texture);
        static RHISampler* getSampler(RHISamplerType const sampler);
        static Mesh* getStandardMesh(geometry::GeometryType const type);

        static math::Vector2 getResolutionRender();
        static math::Vector2 getResolutionOutput();

        static void setPushParameters(float a, float b);
        static void setCameraPosition(math::Vector3 const& position);

    private:
        // =====================================================================
        // Resources
        // =====================================================================
        static void createRasterizerStates();
        static void createDepthStencilStates();
        static void createBlendStates();
        static void createRendererTarget();
        static void createShaders();
        static void createTextures();
        static void createSamplers();
        static void createStandardMeshes();

        static void destroyResources();

        // =====================================================================
        // Passes
        // =====================================================================

        static void passDpethPrepass(RHICommandList* cmdList);
        static void passTest(RHICommandList* cmdList);
        static void passoPostProcessing(RHICommandList* cmdList);

        static void produceFrame(RHICommandList* cmdList);

    private:
        static inline std::uint64_t s_frameCount = 0;
    };

} // namespace worse