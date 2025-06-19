#pragma once
#include "Math/Vector.hpp"
#include "RHIDefinitions.hpp"

#include <cstdint>

namespace worse
{
    class RHIViewport;
    class RHISwapchain;
    class RHICommandList;

    enum class RendererRasterizerState : std::size_t
    {
        Solid,
        Wireframe,
        Max
    };

    enum class RendererDepthStencilState : std::size_t
    {
        Off,
        Max
    };

    enum class RendererBlendState : std::size_t
    {
        Off,
        Max
    };

    enum class RendererShader : std::size_t
    {
        PlaceholderV,
        PlaceholderP,
        PBRV,
        PBRP,
        Max
    };

    enum class RendererTarget : std::size_t
    {
        Render,
        Output,
        Max,
    };

    enum class RendererTexture : std::size_t
    {
        TestA,
        TestB,
        Placeholder,
        Max,
    };

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

        static math::Vector2 getResolutionRender();
        static math::Vector2 getResolutionOutput();

    private:
        static inline std::uint64_t s_frameCount = 0;
    };

} // namespace worse