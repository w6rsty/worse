#include "Pipeline/RHIPipeline.hpp"
#include "RHIDevice.hpp"

namespace worse
{

    RHIPipeline::RHIPipeline(RHIPipelineState const& pipelineState,
                             RHIDescriptorSetLayout const& descriptorSetLayout)
    {
        nativeCreate(pipelineState, descriptorSetLayout);
    }

    RHIPipeline::~RHIPipeline()
    {
        RHIDevice::deletionQueueAdd(m_pipeline);
        m_pipeline = {};

        RHIDevice::deletionQueueAdd(m_pipelineLayout);
        m_pipelineLayout = {};
    }

} // namespace worse