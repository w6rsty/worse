#pragma once
#include "RHIResource.hpp"
#include "RHICommandList.hpp"
#include "RHISyncPrimitive.hpp"

#include <cstdint>

namespace worse
{

    class RHISwapchain : public RHIResource
    {
    public:
        RHISwapchain() = default;
        RHISwapchain(void* sdlWindow, u32 const width, u32 const height,
                     RHIPresentMode const presentMode, std::string_view name);
        ~RHISwapchain();

        void resize(u32 width, u32 height);
        // response window resize event
        void resizeFitWindow();

        void acquireNextImage();
        void present(RHICommandList* cmdList);

        // clang-format off
        u32 getWidth() const                     { return m_width; }
        u32 getHeight() const                    { return m_height; }
        RHIFormat getFormat() const                        { return m_format; }
        RHINativeHandle getCurrentRt() const               { return m_rts[m_imageIndex]; }
        RHINativeHandle getCurrentRtv() const              { return m_rtvs[m_imageIndex]; }
        RHISyncPrimitive* getImageAcquireSemaphore() const { return m_imageAcquireSemaphores[m_imageIndex].get(); }
        // clang-format on

    private:
        // create or recreate
        void create();

    private:
        static inline constexpr u32 s_bufferCount = 2;

        u32 m_width                  = 0;
        u32 m_height                 = 0;
        RHIFormat m_format           = RHIFormat::Max;
        RHIPresentMode m_presentMode = RHIPresentMode::Max;

        void* m_sdlWindow = nullptr;
        bool m_isDirty    = false;
        u32 m_imageIndex  = 0;
        std::array<std::shared_ptr<RHISyncPrimitive>, s_bufferCount * 2>
            m_imageAcquireSemaphores;

        RHINativeHandle m_swapchain;
        RHINativeHandle m_surface;
        // render targets
        std::array<RHINativeHandle, s_bufferCount> m_rts;
        // render target views
        std::array<RHINativeHandle, s_bufferCount> m_rtvs;
    };

} // namespace worse