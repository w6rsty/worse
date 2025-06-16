#include "Log.hpp"
#include "Engine.hpp"
#include "Window.hpp"
#include "Renderer.hpp"
#include "Input/Input.hpp"
#include "Profiling/Stopwatch.hpp"

namespace worse
{
    void Engine::initialize()
    {
#ifdef WS_RHI_BACKEND_VULKAN
        if (volkInitialize() != VK_SUCCESS)
        {
            WS_LOG_ERROR("Engine", "Failed to initialize Vulkan loader.");
            return;
        }
#endif
        profiling::Stopwatch sw;
        {
            WS_LOG_INFO("Engine", "Initializing...");
            Window::initialize();

            Input::initialize();
            Renderer::initialize();
        }
        WS_LOG_INFO("Engine", "Initialization took {:.1f}ms", sw.elapsedMs());
    }

    void Engine::shutdown()
    {
        Renderer::shutdown();
        Window::shutdown();
    }

    void Engine::tick()
    {
        Window::tick();
        Input::tick();

        Renderer::tick();
    }

} // namespace worse