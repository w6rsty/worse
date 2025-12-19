#include "math/hash.hpp"
#include "Pipeline/RHIPipeline.hpp"
#include "RHIDevice.hpp"

namespace Worse
{

    RHIPipeline::RHIPipeline(RHIPipelineState const& pipelineState, RHIDescriptorSetLayout const& descriptorSetLayout)
    {
        // collect ordered set 1 descriptors
        std::vector<RHIDescriptor> descriptors = pipelineState.collectDescriptors();

        ULong hash = 0;
        for (RHIDescriptor const& descriptor : descriptors)
        {
            hash = math::hashCombine(hash, static_cast<ULong>(descriptor.slot));
            hash = math::hashCombine(hash, static_cast<ULong>(descriptor.stageFlags));
        }
        m_descriptorHash = hash;

        nativeCreate(pipelineState, descriptorSetLayout);
    }

    RHIPipeline::~RHIPipeline()
    {
        RHIDevice::deletionQueueAdd(m_pipeline);
        m_pipeline = {};

        RHIDevice::deletionQueueAdd(m_pipelineLayout);
        m_pipelineLayout = {};
    }

    RHIPipelinePool::RHIPipelinePool()
    {
    }

    RHIPipelinePool::~RHIPipelinePool()
    {
        m_pipelines.clear();
    }

    RHIPipeline* RHIPipelinePool::getPipeline(RHIPipelineState const& pso)
    {
        ULong hash = pso.getHash();
        auto it  = m_pipelines.find(hash);
        if (it != m_pipelines.end())
        {
            return it->second.get();
        }

        RHIDescriptorSetLayout* descriptorSetLayout = RHIDevice::getSpecificDescriptorSetLayout(pso);
        WORSE_ASSERT(descriptorSetLayout != nullptr);
        std::shared_ptr<RHIPipeline> pipeline = std::make_shared<RHIPipeline>(pso, *descriptorSetLayout);
        m_pipelines.emplace(hash, pipeline);

        return pipeline.get();
    }

} // namespace Worse