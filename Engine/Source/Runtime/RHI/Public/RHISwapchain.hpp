#pragma once
#include "RHIDefinitions.hpp"
#include "RHICommandList.hpp"
#include "RHIResource.hpp"
#include "RHISyncPrimitive.hpp"

#include <cstdint>

namespace worse
{

    class RHISwapchain
    {
    public:
        RHISwapchain() = default;
        RHISwapchain(void* sdlWindow, std::uint32_t const width,
                     std::uint32_t const height,
                     RHIPresentMode const presentMode,
                     std::uint32_t const bufferCount, char const* name);
        ~RHISwapchain();

        void resize(std::uint32_t width, std::uint32_t height);
        // response window resize event
        void resizeFitWindow();

        void acquireNextImage();
        void present(RHICommandList* cmdList);

        RHIResource getCurrentImage() const;
        RHISyncPrimitive* getImageAcquireSemaphore() const;

    private:
        // create or recreate
        void create();

    private:
        static inline constexpr std::uint32_t s_bufferCount = 2;

        std::uint32_t m_bufferCount  = 0;
        std::uint32_t m_width        = 0;
        std::uint32_t m_height       = 0;
        RHIFormat m_format           = RHIFormat::Max;
        RHIPresentMode m_presentMode = RHIPresentMode::Max;

        void* m_sdlWindow          = nullptr;
        bool m_isDirty             = false;
        std::uint32_t m_imageIndex = 0;
        std::array<std::shared_ptr<RHISyncPrimitive>, s_bufferCount * 2>
            m_imageAcquireSemaphores;

        RHIResource m_rhiSwapchain;
        RHIResource m_rhiSurface;
        std::array<RHIResource, s_bufferCount> m_rhiImages;
        std::array<RHIResource, s_bufferCount> m_rhiImageViews;
    };

} // namespace worse