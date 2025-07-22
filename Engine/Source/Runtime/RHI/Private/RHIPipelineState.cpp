#include "Math/Hash.hpp"
#include "RHIShader.hpp"
#include "RHITexture.hpp"
#include "Pipeline/RHIBlendState.hpp"
#include "Pipeline/RHIPipelineState.hpp"
#include "Pipeline/RHIRasterizerState.hpp"
#include "Pipeline/RHIDepthStencilState.hpp"

namespace worse
{

    namespace
    {
        void validate(RHIPipelineState const& pso)
        {
            u32 width  = 0;
            u32 height = 0;

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
            for (RHIShader const* shader : pso.shaders)
            {
                if ((shader != nullptr) && (shader->getState() == RHIShaderCompilationState::CompiledSuccess))
                {
                    flags |= rhiShaderStageFlags(shader->getShaderType());
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

            WS_ASSERT_MSG(((width != 0) && (height != 0)) || isCompute, "Invalid render target size");

            WS_ASSERT_MSG(!pso.name.empty(), "Pipeline state must have a name");
            // clang-format on
        }

        u64 computeHash(RHIPipelineState const& pso)
        {
            u64 hash = 0;

            hash = math::hashCombine(hash,
                                     static_cast<u64>(pso.primitiveTopology));

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

            for (RHIShader* shader : pso.shaders)
            {
                if (shader)
                {
                    hash = math::hashCombine(hash, shader->getHash());
                }
            }

            for (RHITexture* texture : pso.renderTargetColorTextures)
            {
                if (texture)
                {
                    hash =
                        math::hashCombine(hash, texture->getImage().asValue());
                }
            }

            if (pso.renderTargetDepthTexture)
            {
                hash = math::hashCombine(
                    hash,
                    pso.renderTargetDepthTexture->getImage().asValue());
            }

            return hash;
        }

        // find descriptors bound to the same slot, merge their
        // stageFlags
        void mergeDescriptors(std::vector<RHIDescriptor>& bases,
                              std::vector<RHIDescriptor> const& additionals)
        {
            for (RHIDescriptor const& additional : additionals)
            {
                auto it =
                    std::find_if(bases.begin(),
                                 bases.end(),
                                 [additional](RHIDescriptor const& base)
                                 {
                                     return (base.slot == additional.slot) &&
                                            (base.space == additional.space);
                                 });

                if (it != bases.end())
                {
                    it->stageFlags |= additional.stageFlags;
                }
                else
                {
                    bases.push_back(additional);
                }
            }
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
        return shaders[type];
    }

    std::vector<RHIDescriptor> RHIPipelineState::collectDescriptors() const
    {
        std::vector<RHIDescriptor> descriptors;

        if (type == RHIPipelineType::Compute)
        {
            descriptors = shaders[RHIShaderType::Compute]->getDescriptors();
        }
        else if (type == RHIPipelineType::Graphics)
        {
            descriptors = shaders[RHIShaderType::Vertex]->getDescriptors();
            mergeDescriptors(descriptors,
                             shaders[RHIShaderType::Pixel]->getDescriptors());
        }

        // remove space != 1
        descriptors.erase(
            std::remove_if(descriptors.begin(),
                           descriptors.end(),
                           [](RHIDescriptor const& descriptor)
                           {
                               return (descriptor.space != 1) &&
                                      (descriptor.type !=
                                       RHIDescriptorType::PushConstant);
                           }),
            descriptors.end());

        // helper for generating same hash for same descriptor set with
        // different order
        std::sort(descriptors.begin(),
                  descriptors.end(),
                  [](RHIDescriptor const& a, RHIDescriptor const& b)
                  {
                      return a.slot < b.slot;
                  });
        descriptors.shrink_to_fit();

        return descriptors;
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
        m_pso.shaders[static_cast<usize>(shader->getShaderType())] = shader;
        return *this;
    }

    RHIPipelineStateBuilder&
    RHIPipelineStateBuilder::setRenderTargetColorTexture(usize index,
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

    RHIPipelineStateBuilder& RHIPipelineStateBuilder::setClearDepth(f32 depth)
    {
        m_pso.clearDepth = depth;
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