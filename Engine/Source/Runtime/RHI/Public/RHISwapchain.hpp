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
        RHISwapchain(void* sdlWindow, std::uint32_t const width,
                     std::uint32_t const height,
                     RHIPresentMode const presentMode, std::string_view name);
        ~RHISwapchain();

        void resize(std::uint32_t width, std::uint32_t height);
        // response window resize event
        void resizeFitWindow();

        void acquireNextImage();
        void present(RHICommandList* cmdList);

        // clang-format off
        std::uint32_t getWidth() const                     { return m_width; }
        std::uint32_t getHeight() const                    { return m_height; }
        RHIFormat getFormat() const                        { return m_format; }
        RHINativeHandle getCurrentRt() const               { return m_rts[m_imageIndex]; }
        RHINativeHandle getCurrentRtv() const              { return m_rtvs[m_imageIndex]; }
        RHISyncPrimitive* getImageAcquireSemaphore() const { return m_imageAcquireSemaphores[m_imageIndex].get(); }
        // clang-format on

    private:
        // create or recreate
        void create();

    private:
        static inline constexpr std::uint32_t s_bufferCount = 2;

        std::uint32_t m_width        = 0;
        std::uint32_t m_height       = 0;
        RHIFormat m_format           = RHIFormat::Max;
        RHIPresentMode m_presentMode = RHIPresentMode::Max;

        void* m_sdlWindow          = nullptr;
        bool m_isDirty             = false;
        std::uint32_t m_imageIndex = 0;
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