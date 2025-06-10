#include "Math/Hash.hpp"
#include "RHIShader.hpp"
#include "Pipeline/RHIPipelineState.hpp"
#include "Pipeline/RHIRasterizerState.hpp"
#include "Pipeline/RHIDepthStencilState.hpp"
#include "Pipeline/RHIBlendState.hpp"
#include "Descriptor/RHITexture.hpp"

namespace worse
{

    namespace
    {
        void validate(RHIPipelineState const& pso)
        {
            std::uint32_t width  = 0;
            std::uint32_t height = 0;

            if (pso.renderTargetColorTextures[0])
            {
                width  = pso.renderTargetColorTextures[0]->getWidth();
                height = pso.renderTargetColorTextures[0]->getHeight();
            }
            else if (pso.renderTargetDepthTexture)
            {
                width  = pso.renderTargetDepthTexture->getWidth();
                height = pso.renderTargetDepthTexture->getHeight();
            }

            // clang-format off
            RHIShaderStageFlags flags = {};
            for (std::size_t i = 0; i < k_rhiShaderTypeCount; ++i)
            {
                if ((pso.shaders[i] != nullptr) && (pso.shaders[i]->getState() == RHIShaderCompilationState::CompiledSuccess))
                {
                    flags |= rhiShaderStageFlags(pso.shaders[i]->getShaderType());
                }
            }

            bool isCompute = (flags & RHIComputePipelineShaderCombination) == RHIComputePipelineShaderCombination;
            bool isGraphics = (flags & RHIGraphicsPipelineShaderCombination) == RHIGraphicsPipelineShaderCombination;
            WS_ASSERT_MSG(isCompute || isGraphics, "Invalid shader set");

            if (isCompute)
            {
                WS_ASSERT_MSG(pso.type == RHIPipelineType::Compute, "Incompatible pipeline type");
            }
            
            if (isGraphics)
            {
                WS_ASSERT_MSG(pso.type == RHIPipelineType::Graphics, "Incompatible pipeline type");

                bool hasRenderTarget = (pso.renderTargetColorTextures[0] != nullptr) || pso.renderTargetDepthTexture;

                WS_ASSERT_MSG(hasRenderTarget, "Pipeline has no render target");

                bool hasMandatoryState = (pso.rasterizerState != nullptr) && (pso.depthStencilState != nullptr) && (pso.blendState != nullptr);
                WS_ASSERT_MSG(hasMandatoryState, "Graphics Pipeline miss mandatory states");
            }

            WS_ASSERT_MSG((width != 0) && (height != 0), "Invalid render target size");

            WS_ASSERT_MSG(!pso.name.empty(), "Pipeline state must have a name");
            // clang-format on
        }

        std::uint64_t computeHash(RHIPipelineState const& pso)
        {
            std::uint64_t hash = 0;

            if (pso.rasterizerState)
            {
                hash = math::hashCombine(hash, pso.rasterizerState->getHash());
            }

            if (pso.depthStencilState)
            {
                hash =
                    math::hashCombine(hash, pso.depthStencilState->getHash());
            }

            if (pso.blendState)
            {
                hash = math::hashCombine(hash, pso.blendState->getHash());
            }

            return hash;
        }
    } // namespace

    RHIPipelineState::~RHIPipelineState()
    {
    }

    void RHIPipelineState::finalize()
    {
        validate(*this);

        m_hash = computeHash(*this);

        // if not valid, assertion will be triggered before
        m_validated = true;
    }

    RHIShader const* RHIPipelineState::getShader(RHIShaderType const type) const
    {
        return shaders[static_cast<std::size_t>(type)];
    }

    RHIPipelineStateBuilder&
    RHIPipelineStateBuilder::setName(std::string const& name)
    {

        m_pso.name = name;
        return *this;
    }

    RHIPipelineStateBuilder&
    RHIPipelineStateBuilder::setType(RHIPipelineType type)
    {
        m_pso.type = type;
        return *this;
    }

    RHIPipelineStateBuilder&
    RHIPipelineStateBuilder::setPrimitiveTopology(RHIPrimitiveTopology topology)
    {
        m_pso.primitiveTopology = topology;
        return *this;
    }

    RHIPipelineStateBuilder&
    RHIPipelineStateBuilder::setRasterizerState(RHIRasterizerState* state)
    {
        m_pso.rasterizerState = state;
        return *this;
    }

    RHIPipelineStateBuilder&
    RHIPipelineStateBuilder::setDepthStencilState(RHIDepthStencilState* state)
    {
        m_pso.depthStencilState = state;
        return *this;
    }

    RHIPipelineStateBuilder&
    RHIPipelineStateBuilder::setBlendState(RHIBlendState* state)
    {
        m_pso.blendState = state;
        return *this;
    }

    RHIPipelineStateBuilder&
    RHIPipelineStateBuilder::addShader(RHIShader* shader)
    {
        m_pso.shaders[static_cast<std::size_t>(shader->getShaderType())] =
            shader;
        return *this;
    }

    RHIPipelineStateBuilder&
    RHIPipelineStateBuilder::setRenderTargetColorTexture(std::size_t index,
                                                         RHITexture* texture)
    {
        m_pso.renderTargetColorTextures[index] = texture;
        return *this;
    }

    RHIPipelineStateBuilder&
    RHIPipelineStateBuilder::setRenderTargetDepthTexture(RHITexture* texture)
    {
        m_pso.renderTargetDepthTexture = texture;
        return *this;
    }

    RHIPipelineStateBuilder&
    RHIPipelineStateBuilder::setScissor(math::Rectangle const& scissor)
    {
        m_pso.scissor = scissor;
        return *this;
    }

    RHIPipelineStateBuilder&
    RHIPipelineStateBuilder::setViewport(RHIViewport const& viewport)
    {
        m_pso.viewport = viewport;
        return *this;
    }

    RHIPipelineStateBuilder&
    RHIPipelineStateBuilder::setClearColor(Color const& color)
    {
        m_pso.clearColor = color;
        return *this;
    }

    RHIPipelineState RHIPipelineStateBuilder::build()
    {
        m_pso.finalize();

        return std::move(m_pso);
    }

} // namespace worse