#pragma once
#include "Math/Vector2.hpp"
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
    constexpr std::size_t k_rendererRasterizerStateCount =
        static_cast<std::size_t>(RendererRasterizerState::Max);

    enum class RendererDepthStencilState : std::size_t
    {
        Off,
        Max
    };
    constexpr std::size_t k_rendererDepthStencilStateCount =
        static_cast<std::size_t>(RendererDepthStencilState::Max);

    enum class RendererBlendState : std::size_t
    {
        Off,
        Max
    };
    constexpr std::size_t k_rendererBlendStateCount =
        static_cast<std::size_t>(RendererBlendState::Max);

    enum class RendererShader : std::size_t
    {
        QuadV,
        QuadP,
        Max
    };
    constexpr std::size_t k_rendererShaderCount =
        static_cast<std::size_t>(RendererShader::Max);

    enum class RendererTarget : std::size_t
    {
        Render,
        Output,
        Max,
    };
    constexpr std::size_t k_rendererTargetCount =
        static_cast<std::size_t>(RendererTarget::Max);

    class Renderer
    {
        static void createRasterizerStates();
        static void createDepthStencilStates();
        static void createBlendStates();
        static void createShaders();
        static void createRendererTarget();

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

        static math::Vector2 getResolutionRender();
        static math::Vector2 getResolutionOutput();
    private:
        static inline std::uint64_t s_frameCount = 0;
    };

} // namespace worse