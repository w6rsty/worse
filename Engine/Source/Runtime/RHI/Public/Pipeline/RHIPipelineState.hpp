#pragma once
#include "Math/Rectangle.hpp"
#include "RHIDefinitions.hpp"
#include "RHIViewport.hpp"
#include "Color.hpp"

namespace worse
{

    class RHIPipelineState
    {
    public:
        RHIPipelineState() = default;
        ~RHIPipelineState();

        // making sure shaders are compiled, and states are complete,
        // and generate hash
        void finalize();
        RHIShader const* getShader(RHIShaderType const type) const;

        // clang-format off
        bool isValidated() const      { return m_validated; }
        std::uint32_t getHash() const { return m_hash; }
        // clang-format on

        std::string name = "pso";

        // clang-format off
        RHIPipelineType type                                 = RHIPipelineType::Graphics;
        RHIPrimitiveTopology primitiveTopology               = RHIPrimitiveTopology::Trianglelist;
        RHIRasterizerState* rasterizerState                  = nullptr;
        RHIDepthStencilState* depthStencilState              = nullptr;
        RHIBlendState* blendState                            = nullptr;
        std::array<RHIShader*, k_rhiShaderTypeCount> shaders = {};
        RHISwapchain* renderTargetSwapchain = nullptr;
        std::array<RHITexture*, RHIConfig::MAX_RENDER_TARGET> renderTargetColorTextures = {nullptr};
        RHITexture* renderTargetDepthTexture = nullptr;
        // clang-format on

        math::Rectangle scissor = {};
        RHIViewport viewport    = {};
        Color clearColor        = Color::Black();

    private:
        bool m_validated     = false;
        std::uint64_t m_hash = 0;
    };

} // namespace worse