#pragma once
#include "math/rectangle.hpp"
#include "RHIDefinitions.hpp"
#include "RHIResource.hpp"
#include "RHIViewport.hpp"
#include "RHIDescriptor.hpp"
#include "Pipeline/RHIPipelineState.hpp"

#include <span>
#include <atomic>

namespace Worse
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
        UInt aspectMask          = 0;
        UInt mipIndex            = 0;
        UInt mipRange            = 0;
        UInt arrayLength         = 0;
        RHIImageLayout layoutOld = RHIImageLayout::Max;
        RHIImageLayout layoutNew = RHIImageLayout::Max;
        Bool isDepth             = false;
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

        void draw(UInt const vertexCount, UInt const vertexOffset = 0);
        void drawIndexed(UInt const indexCount, UInt const indexOffset = 0,
                         UInt const vertexOffset  = 0,
                         UInt const instanceIndex = 0,
                         UInt const instanceCount = 1);

        void dispatch(UInt const x, UInt const y, UInt const z = 1);

        // bind pipeline specific resources and begin render pass make sure pso
        // has been called `finalize()`
        void setPipelineState(RHIPipelineState const& pso);
        // must call this at the end of the render operation if only has one pso
        // in render loop, otherwise the activation of next pass will fail
        void clearPipelineState();

        // dynamic state
        void setViewport(RHIViewport const& viewport);
        void setScissor(math::Rectangle const& scissor);

        void insertBarrier(
            RHINativeHandle const image,
            RHIFormat const format,
            RHIImageLayout const layoutNew,
            RHIPipelineStage::Flags const srcStage = RHIPipelineStage::FlagBits::AllCommands,
            RHIAccessUsage::Flags const srcAccess  = RHIAccessUsage::FlagBits::MemoryRead,
            RHIPipelineStage::Flags const dstStage = RHIPipelineStage::FlagBits::AllCommands,
            RHIAccessUsage::Flags const dstAccess  = RHIAccessUsage::FlagBits::MemoryWrite | RHIAccessUsage::FlagBits::MemoryWrite);

        void blit(RHITexture const* source, RHITexture const* destination);
        void blit(RHITexture const* source, RHISwapchain const* destination);

        void copy(RHITexture const* source, RHITexture const* destination);
        void copy(RHITexture const* source, RHISwapchain const* destination);

        void pushConstants(std::span<Byte, RHIConfig::MAX_PUSH_CONSTANT_SIZE> data);

        void setBufferVertex(RHIBuffer* buffer);
        void setBufferIndex(RHIBuffer* buffer);

        void updateBuffer(RHIBuffer* buffer, UInt const offset, UInt const size, void const* data);

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
        Bool m_isFirstGraphicsPass = true;
        RHIPipelineState m_pso;
        RHIPipeline* m_pipeline = nullptr;

        std::atomic<RHICommandListState> m_state = RHICommandListState::Idle;
        RHIQueue* m_submissionQueue              = nullptr;
        RHINativeHandle m_handle; // VkCommandBuffer

        Bool m_isRenderPassActive = false;
    };

} // namespace Worse