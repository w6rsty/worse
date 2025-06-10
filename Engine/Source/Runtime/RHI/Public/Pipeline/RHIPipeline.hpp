#pragma once
#include "RHIResource.hpp"
#include "RHIPipelineState.hpp"
#include "Descriptor/RHIDescriptorSetLayout.hpp"

namespace worse
{

    class RHIPipeline : public RHIResource
    {
        void nativeCreate(RHIPipelineState const& pipelineState,
                          RHIDescriptorSetLayout const& descriptorSetLayout);

    public:
        RHIPipeline(RHIPipelineState const& pipelineState,
                    RHIDescriptorSetLayout const& descriptorSetLayout);
        ~RHIPipeline();

        // clang-format off
        RHIPipelineState* getState()              { return &m_state; }
        RHINativeHandle getHandle() const       { return m_pipeline; }
        RHINativeHandle getLayout() const { return m_pipelineLayout; }
        // clang-format on

    private:
        RHIPipelineState m_state;
        RHINativeHandle m_pipeline;
        RHINativeHandle m_pipelineLayout;
    };
} // namespace worse