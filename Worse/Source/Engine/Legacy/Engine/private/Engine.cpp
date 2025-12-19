#include "logger/logger_macro.hpp"
#include "Engine.hpp"
#include "Window.hpp"
#include "Input/Input.hpp"
#include "RHIDefinitions.hpp"

namespace Worse
{
    void Engine::initialize(ecs::Commands commands)
    {
#ifdef WS_RHI_BACKEND_VULKAN
        if (volkInitialize() != VK_SUCCESS)
        {
            WORSE_LOG_ERROR("Engine", "Failed to initialize Vulkan loader.");
            return;
        }
#endif

        WORSE_LOG_INFO("Engine", "Initializing...");
        Window::initialize();
        Input::initialize();
    }

    void Engine::tick(ecs::Commands commands)
    {
        Window::tick();
        Input::tick();
    }

    void Engine::shutdown()
    {
        Window::shutdown();
    }

} // namespace Worse