#pragma once

namespace worse
{
    class RHIViewport;
    class RHISwapchain;
    class RHICommandList;

    class Renderer
    {
    public:
        static void initialize();
        static void shutdown();
        static void tick();

        // swapchain
        static RHISwapchain* getSwapchain();
        // submit command list and present image to swapchain
        static void submitAndPresent();

        static void setViewport(float const width, float const height);
        static RHIViewport const& getViewport();

    private:
    };

} // namespace worse