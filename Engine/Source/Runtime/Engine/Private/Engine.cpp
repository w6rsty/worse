#include "Log.hpp"
#include "Engine.hpp"
#include "Math/Vector.hpp"
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

        {
            static float radius = 2.0f;
            if (Input::isKeyDown(KeyCode::ArrowUp))
            {
                radius += 1.0f;
            }
            if (Input::isKeyDown(KeyCode::ArrowDown))
            {
                radius -= 1.0f;
            }
            Renderer::setPushParameters(radius, 0.0f);
        }
        {
            static math::Vector3 cameraPosition = {0.0f, 0.5f, 1.0f};
            if (Input::isKey(KeyCode::W))
            {
                cameraPosition.z -= 0.01f;
            }
            if (Input::isKey(KeyCode::S))
            {
                cameraPosition.z += 0.01f;
            }
            if (Input::isKey(KeyCode::A))
            {
                cameraPosition.x -= 0.01f;
            }
            if (Input::isKey(KeyCode::D))
            {
                cameraPosition.x += 0.01f;
            }
            if (Input::isKey(KeyCode::Space))
            {
                cameraPosition.y += 0.01f;
            }
            if (Input::isKey(KeyCode::ShiftLeft))
            {
                cameraPosition.y -= 0.01f;
            }

            Renderer::setCameraPosition(cameraPosition);
        }

        Renderer::tick();
    }

} // namespace worse