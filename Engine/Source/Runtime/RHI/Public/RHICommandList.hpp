#pragma once
#include "Math/Rectangle.hpp"
#include "RHIDefinitions.hpp"
#include "RHIResource.hpp"
#include "RHIViewport.hpp"
#include "Pipeline/RHIPipelineState.hpp"
#include "Descriptor/RHIDescriptorSetLayout.hpp"

#include <cstdint>
#include <atomic>

namespace worse
{

    enum class RHICommandListState
    {
        Idle,
        Recording,
        Submitted,
    };

    struct ImageBarrierInfo
    {
        void* image               = nullptr;
        std::uint32_t aspectMask  = 0;
        std::uint32_t mipIndex    = 0;
        std::uint32_t mipRange    = 0;
        std::uint32_t arrayLength = 0;
        RHIImageLayout layoutOld  = RHIImageLayout::Max;
        RHIImageLayout layoutNew  = RHIImageLayout::Max;
        bool isDepth              = false;
    };

    class RHICommandList : public RHIResource
    {
    public:
        RHICommandList(RHIQueue* queue, RHINativeHandle cmdPool,
                       std::string_view name);
        ~RHICommandList();

        void begin();
        void submit(RHISyncPrimitive* semaphoreWait);
        void waitForExecution();

        void renderPassBegin();
        void renderPassEnd();

        void draw(std::uint32_t const vertexCount,
                  std::uint32_t const vertexOffset = 0);
        void drawIndexed(std::uint32_t const indexCount,
                         std::uint32_t const indexOffset   = 0,
                         std::uint32_t const vertexOffset  = 0,
                         std::uint32_t const instanceIndex = 0,
                         std::uint32_t const instanceCount = 1);

        void dipatch(std::uint32_t const x, std::uint32_t const y,
                     std::uint32_t const z = 1);

        // bind pipeline specific resources and begin render pass make sure pso
        // has been called `finalize()`
        void setPipelineState(RHIPipelineState const& pso);
        // must call this at the end of the render operation if only has one pso
        // in reneder loop, otherwise the activation of next pass will fail
        void clearPipelineState();

        void setViewport(RHIViewport const& viewport);
        void setScissor(math::Rectangle const& scissor);

        void insertBarrier(RHINativeHandle const image, RHIFormat const format,
                           RHIImageLayout const layoutNew);

        void blit(RHITexture const* source, RHITexture const* destination);
        void blit(RHITexture const* source, RHISwapchain const* destination);

        void copy(RHITexture const* source, RHITexture const* destination);
        void copy(RHITexture const* source, RHISwapchain const* destination);

        void setBufferVertex(RHIBuffer* buffer);
        void setBufferIndex(RHIBuffer* buffer);
        // need to update descriptor set
        void setContantBuffer(RHIBuffer* buffer, std::uint32_t const slot);
        // need to update descriptor set
        void setBuffer(RHIBuffer* buffer, std::uint32_t const slot);

        void updateBuffer(RHIBuffer* buffer, std::uint32_t const offset,
                          std::uint32_t const size, void const* data);

        void bindSet(RHIBindlessResourceType const type,
                     RHINativeHandle const set);

        // clang-format off
        RHISyncPrimitive* getRenderingCompleteSemaphore() { return m_renderingCompleteBinaySemaphore.get(); }
        RHICommandListState getState() const              { return m_state; }
        RHIQueue* getQueue() const                        { return m_submissionQueue; }
        RHINativeHandle getHandle() const                 { return m_handle; }
        // clang-format on

        // TODO: move to other place
        static RHIImageLayout getImageLayout(RHINativeHandle image);

    private:
        std::shared_ptr<RHISyncPrimitive> m_renderingCompleteBinaySemaphore;
        std::shared_ptr<RHISyncPrimitive> m_renderingCompleteTimelineSemaphore;

        RHIPipelineState m_pso;
        RHIPipeline* m_pipeline                       = nullptr;
        RHIDescriptorSetLayout* m_descriptorSetLayout = nullptr;

        std::atomic<RHICommandListState> m_state = RHICommandListState::Idle;
        RHIQueue* m_submissionQueue              = nullptr;
        RHINativeHandle m_handle;

        bool m_isRenderPassActive = false;
    };

} // namespace worse