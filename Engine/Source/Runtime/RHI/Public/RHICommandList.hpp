#pragma once
#include "Math/Rectangle.hpp"
#include "RHIDefinitions.hpp"
#include "RHIResource.hpp"
#include "RHIViewport.hpp"
#include "RHIDescriptor.hpp"
#include "Pipeline/RHIPipelineState.hpp"

#include <span>
#include <atomic>

namespace worse
{

    // track submission state
    enum class RHICommandListState
    {
        Idle,
        Recording,
        Submitted,
    };

    struct ImageBarrierInfo
    {
        void* image              = nullptr;
        u32 aspectMask           = 0;
        u32 mipIndex             = 0;
        u32 mipRange             = 0;
        u32 arrayLength          = 0;
        RHIImageLayout layoutOld = RHIImageLayout::Max;
        RHIImageLayout layoutNew = RHIImageLayout::Max;
        bool isDepth             = false;
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

        void imguiPassBegin(RHITexture const* renderTarget,
                            math::Rectangle const& scissor);
        void imguiPassEnd(void* drawData);

        void draw(u32 const vertexCount, u32 const vertexOffset = 0);
        void drawIndexed(u32 const indexCount, u32 const indexOffset = 0,
                         u32 const vertexOffset  = 0,
                         u32 const instanceIndex = 0,
                         u32 const instanceCount = 1);

        void dispatch(u32 const x, u32 const y, u32 const z = 1);

        // bind pipeline specific resources and begin render pass make sure pso
        // has been called `finalize()`
        void setPipelineState(RHIPipelineState const& pso);
        // must call this at the end of the render operation if only has one pso
        // in reneder loop, otherwise the activation of next pass will fail
        void clearPipelineState();

        // dynamic state
        void setViewport(RHIViewport const& viewport);
        void setScissor(math::Rectangle const& scissor);

        void insertBarrier(
            RHINativeHandle const image,
            RHIFormat const format,
            RHIImageLayout const layoutNew,
            RHIPipelineStageFlags const srcStage = RHIPipelineStageFlagBits::AllCommands,
            RHIAccessFlags const srcAccess       = RHIAccessFlagBits::MemoryRead,
            RHIPipelineStageFlags const dstStage = RHIPipelineStageFlagBits::AllCommands,
            RHIAccessFlags const dstAccess       = RHIAccessFlagBits::MemoryWrite | RHIAccessFlagBits::MemoryWrite);

        void blit(RHITexture const* source, RHITexture const* destination);
        void blit(RHITexture const* source, RHISwapchain const* destination);

        void copy(RHITexture const* source, RHITexture const* destination);
        void copy(RHITexture const* source, RHISwapchain const* destination);

        void pushConstants(std::span<byte, RHIConfig::MAX_PUSH_CONSTANT_SIZE> data);

        void setBufferVertex(RHIBuffer* buffer);
        void setBufferIndex(RHIBuffer* buffer);

        void updateBuffer(RHIBuffer* buffer, u32 const offset, u32 const size, void const* data);

        // get global set 0 and bind
        void bindGlobalSet();
        // get pipeline specific set 1, or create one if not exists, and bind
        void bindSpecificSet();
        void updateSpecificSet(std::span<RHIDescriptorWrite> writes);

        // clang-format off
        RHISyncPrimitive*   getRenderingCompleteSemaphore() { return m_renderingCompleteBinaySemaphore.get(); }
        RHICommandListState getState() const                { return m_state; }
        RHIQueue*           getQueue() const                { return m_submissionQueue; }
        RHINativeHandle     getHandle() const               { return m_handle; }
        // clang-format on

        // TODO: move to other place
        static RHIImageLayout getImageLayout(RHINativeHandle image);

    private:
        std::shared_ptr<RHISyncPrimitive> m_renderingCompleteBinaySemaphore;
        std::shared_ptr<RHISyncPrimitive> m_renderingCompleteTimelineSemaphore;

        // for bind global descriptor set once
        bool m_isFirstGraphicsPass = true;
        bool m_isFirstComputePass  = true;
        RHIPipelineState m_pso;
        RHIPipeline* m_pipeline = nullptr;

        std::atomic<RHICommandListState> m_state = RHICommandListState::Idle;
        RHIQueue* m_submissionQueue              = nullptr;
        RHINativeHandle m_handle; // VkCommandBuffer

        bool m_isRenderPassActive = false;
    };

} // namespace worse