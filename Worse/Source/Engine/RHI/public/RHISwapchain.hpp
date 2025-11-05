#pragma once
#include "RHIResource.hpp"
#include "RHICommandList.hpp"
#include "RHISyncPrimitive.hpp"

namespace Worse
{

    class RHISwapchain : public RHIResource
    {
    public:
        RHISwapchain() = default;
        RHISwapchain(void* sdlWindow, UInt const width, UInt const height,
                     RHIPresentMode const presentMode, std::string_view name);
        ~RHISwapchain();

        void resize(UInt width, UInt height);
        // response window resize event
        void resizeFitWindow();

        void acquireNextImage();
        void present(RHICommandList* cmdList);

        // clang-format off
        UInt getWidth() const                     { return m_width; }
        UInt getHeight() const                    { return m_height; }
        RHIFormat getFormat() const                        { return m_format; }
        RHINativeHandle getCurrentRt() const               { return m_rts[m_imageIndex]; }
        RHINativeHandle getCurrentRtv() const              { return m_rtvs[m_imageIndex]; }
        RHISyncPrimitive* getImageAcquireSemaphore() const { return m_imageAcquireSemaphores[m_imageIndex].get(); }
        // clang-format on

    private:
        // create or recreate
        void create();

    private:
        static inline constexpr UInt s_bufferCount = 2;

        UInt m_width                 = 0;
        UInt m_height                = 0;
        RHIFormat m_format           = RHIFormat::Max;
        RHIPresentMode m_presentMode = RHIPresentMode::Max;

        void* m_sdlWindow = nullptr;
        Bool m_isDirty    = false;
        UInt m_imageIndex = 0;
        std::array<std::shared_ptr<RHISyncPrimitive>, s_bufferCount * 2>
            m_imageAcquireSemaphores;

        RHINativeHandle m_swapchain;
        RHINativeHandle m_surface;
        // render targets
        std::array<RHINativeHandle, s_bufferCount> m_rts;
        // render target views
        std::array<RHINativeHandle, s_bufferCount> m_rtvs;
    };

} // namespace Worse