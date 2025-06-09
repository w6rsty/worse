#pragma once
#include "Math/Rectangle.hpp"
#include "RHIResource.hpp"
#include "RHIViewport.hpp"
#include "Pipeline/RHIPipelineState.hpp"

#include <cstdint>
#include <atomic>

namespace worse
{

    class RHIQueue;
    class RHISyncPrimitive;
    class RHISwapchain;

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

        void insertBarrier(RHINativeHandle image, RHIFormat const format,
                           RHIImageLayout const layoutNew);

        void blit(RHITexture* source, RHITexture* destination);
        void blit(RHITexture* source, RHISwapchain* destination);

        void copy(RHITexture* source, RHITexture* destination);
        void copy(RHITexture* source, RHISwapchain* destination);

        RHISyncPrimitive* getRenderingCompleteSemaphore();
        RHICommandListState getState() const;
        RHIQueue* getQueue() const;

        // TODO: move to other place
        static RHIImageLayout getImageLayout(RHINativeHandle image);

    private:
        std::shared_ptr<RHISyncPrimitive> m_renderingCompleteBinaySemaphore;
        std::shared_ptr<RHISyncPrimitive> m_renderingCompleteTimelineSemaphore;

        RHIPipelineState m_pso;
        std::atomic<RHICommandListState> m_state = RHICommandListState::Idle;
        RHIQueue* m_submissionQueue              = nullptr;
        RHINativeHandle m_handle;

        bool m_isRenderPassActive = false;
    };

} // namespace worse