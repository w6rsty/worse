#pragma once
#include "RHIResource.hpp"
#include "RHIPipelineState.hpp"
#include "Descriptor/RHIDescriptorSetLayout.hpp"

#include <cstdint>
#include <unordered_map>

namespace worse
{

    class RHIPipeline : public RHIResource
    {
        void nativeCreate(RHIPipelineState const& pipelineState,
                          RHIDescriptorSetLayout const& descriptorSetLayout);

    public:
        RHIPipeline(
            RHIPipelineState const& pipelineState,
            RHIDescriptorSetLayout const& descriptorSetLayout);
        ~RHIPipeline();

        // clang-format off
        std::uint64_t     getDescriptorHash() const { return m_descriptorHash; }
        RHIPipelineState* getState()                { return &m_state; }
        RHINativeHandle   getHandle() const         { return m_pipeline; }
        RHINativeHandle   getLayout() const         { return m_pipelineLayout; }
        // clang-format on

    private:
        // use this hash to retrieve descriptor set layout
        std::uint64_t m_descriptorHash = 0;

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
        std::unordered_map<std::uint64_t, std::shared_ptr<RHIPipeline>> m_pipelines;
        // clang-format on
    };

} // namespace worse