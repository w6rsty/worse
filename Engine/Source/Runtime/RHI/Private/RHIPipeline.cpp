#include "Math/Hash.hpp"
#include "Pipeline/RHIPipeline.hpp"
#include "RHIDevice.hpp"

namespace worse
{

    RHIPipeline::RHIPipeline(RHIPipelineState const& pipelineState,
                             RHIDescriptorSetLayout const& descriptorSetLayout)
    {
        // collect ordered set 1 descriptors
        std::vector<RHIDescriptor> descriptors =
            pipelineState.collectDescriptors();

        std::uint64_t hash = 0;
        for (RHIDescriptor const& descriptor : descriptors)
        {
            // clang-format off
            hash = math::hashCombine(hash, static_cast<std::uint64_t>(descriptor.slot));
            hash = math::hashCombine(hash, static_cast<std::uint64_t>(descriptor.stageFlags));
            // clang-format on
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
        std::uint64_t hash = pso.getHash();
        auto it            = m_pipelines.find(hash);
        if (it != m_pipelines.end())
        {
            return it->second.get();
        }

        // clang-format off
        RHIDescriptorSetLayout* descriptorSetLayout = RHIDevice::getSpecificDescriptorSetLayout(pso);
        WS_ASSERT(descriptorSetLayout != nullptr);
        std::shared_ptr<RHIPipeline> pipeline = std::make_shared<RHIPipeline>(pso, *descriptorSetLayout);
        // clang-format on
        m_pipelines.emplace(hash, pipeline);

        return pipeline.get();
    }

} // namespace worse