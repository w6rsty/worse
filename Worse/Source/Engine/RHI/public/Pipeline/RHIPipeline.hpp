#pragma once
#include "RHIResource.hpp"
#include "RHIPipelineState.hpp"
#include "RHIDescriptorSetLayout.hpp"

#include <unordered_map>

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
        u64     getDescriptorHash() const { return m_descriptorHash; }
        RHIPipelineState* getState()                { return &m_state; }
        RHINativeHandle   getHandle() const         { return m_pipeline; }
        RHINativeHandle   getLayout() const         { return m_pipelineLayout; }
        // clang-format on

    private:
        // use this hash to retrieve descriptor set layout
        u64 m_descriptorHash = 0;

        RHIPipelineState m_state;
        RHINativeHandle m_pipeline;
        RHINativeHandle m_pipelineLayout;
    };

    class RHIPipelinePool : public NonCopyable
    {
    public:
        RHIPipelinePool();
        ~RHIPipelinePool();

        // get pipeline from pool or create a new one
        RHIPipeline* getPipeline(RHIPipelineState const& pso);

    private:
        // clang-format off
        // hash by pso
        std::unordered_map<u64, std::shared_ptr<RHIPipeline>> m_pipelines;
        // clang-format on
    };

} // namespace worse