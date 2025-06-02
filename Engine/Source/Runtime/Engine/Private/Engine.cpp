#include "Engine.hpp"
#include "Window.hpp"
#include "Log.hpp"
#include "Input/Input.hpp"
#include "Profiling/Stopwatch.hpp"
#include "Renderer.hpp"
#include "RHIDefinitions.hpp"

namespace worse
{
    void Engine::initialize()
    {

        profiling::Stopwatch sw;
        {
#ifdef WS_RHI_BACKEND_VULKAN
            if (volkInitialize() != VK_SUCCESS)
            {
                WS_LOG_FATAL("RHI", "System does not support Vulkan");
                return;
            }
#endif
            log::Logger::initialize();
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
        log::Logger::shutdown();
    }

    void Engine::tick()
    {
        // static std::uint64_t frameCount = 0;
        // WS_LOG_DEBUG("Engine", "Frame: {}", frameCount++);
        Window::tick();
        Input::tick();

        Renderer::tick();
    }

} // namespace worse