#pragma once
#include "Math/Vector.hpp"
#include "RHIDefinitions.hpp"
#include "RendererDefinitions.hpp"

#include <cstdint>

namespace worse
{

    class Renderer
    {
        static void createRasterizerStates();
        static void createDepthStencilStates();
        static void createBlendStates();
        static void createRendererTarget();
        static void createShaders();
        static void createTextures();
        static void createSamplers();

        static void destroyResources();

    public:
        static void initialize();
        static void shutdown();
        static void tick();

        // swapchain
        static RHISwapchain* getSwapchain();
        static void blitToBackBuffer(RHICommandList* cmdList);
        // submit command list and present image to swapchain
        static void submitAndPresent();

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

        static Vector2 getResolutionRender();
        static Vector2 getResolutionOutput();

    private:
        static inline std::uint64_t s_frameCount = 0;
    };

} // namespace worse