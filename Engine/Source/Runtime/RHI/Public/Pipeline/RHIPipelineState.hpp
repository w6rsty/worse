#pragma once
#include "Math/Rectangle.hpp"
#include "Color.hpp"
#include "RHIDefinitions.hpp"
#include "RHIViewport.hpp"

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
        std::vector<RHIDescriptor> collectDescriptors() const;

        // clang-format off
        bool isValidated() const      { return m_validated; }
        std::uint64_t getHash() const { return m_hash; }
        // clang-format on

        std::string name = "pso";

        // clang-format off
        RHIPipelineType type                                 = RHIPipelineType::Graphics;
        RHIPrimitiveTopology primitiveTopology               = RHIPrimitiveTopology::TriangleList;
        RHIRasterizerState* rasterizerState                  = nullptr;
        RHIDepthStencilState* depthStencilState              = nullptr;
        RHIBlendState* blendState                            = nullptr;
        EnumArray<RHIShaderType, RHIShader*> shaders= {};
        std::array<RHITexture*, RHIConfig::MAX_RENDER_TARGET> renderTargetColorTextures = {nullptr};
        RHITexture* renderTargetDepthTexture = nullptr;
        // clang-format on

        math::Rectangle scissor = {};
        RHIViewport viewport    = {};
        float clearDepth        = std::numeric_limits<float>::max();
        Color clearColor        = Color::Black();

    private:
        bool m_validated     = false;
        std::uint64_t m_hash = 0;
    };

    class RHIPipelineStateBuilder
    {
    public:
        RHIPipelineStateBuilder() = default;

        // clang-format off
        RHIPipelineStateBuilder& setName(std::string const& name);
        RHIPipelineStateBuilder& setType(RHIPipelineType type);
        RHIPipelineStateBuilder& setPrimitiveTopology(RHIPrimitiveTopology topology);
        RHIPipelineStateBuilder& setRasterizerState(RHIRasterizerState* state);
        RHIPipelineStateBuilder& setDepthStencilState(RHIDepthStencilState* state);
        RHIPipelineStateBuilder& setBlendState(RHIBlendState* state);
        RHIPipelineStateBuilder& addShader(RHIShader* shader);
        RHIPipelineStateBuilder& setRenderTargetColorTexture(std::size_t index, RHITexture* texture);
        RHIPipelineStateBuilder& setRenderTargetDepthTexture(RHITexture* texture);
        RHIPipelineStateBuilder& setScissor(math::Rectangle const& scissor);
        RHIPipelineStateBuilder& setViewport(RHIViewport const& viewport);
        RHIPipelineStateBuilder& setClearDepth(float depth);
        RHIPipelineStateBuilder& setClearColor(Color const& color);
        // clang-format on

        RHIPipelineState build();

    private:
        RHIPipelineState m_pso;
    };

} // namespace worse
