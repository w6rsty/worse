#include "Engine.hpp"
#include "Window.hpp"
#include "Input/Input.hpp"
#include "Input/Controller.hpp"
#include "Log.hpp"

#include <thread>

using namespace worse;

struct Color
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

constexpr Color k_health{0, 255, 0};
constexpr Color k_injured{255, 255, 0};
constexpr Color k_dying{255, 0, 0};

int main()
{
    bool mouseVisible = true;

    Engine::initialize();

    while (!Window::shouldClose())
    {
        Engine::tick();

        if (Input::isKeyDown(KeyCode::Escape))
        {
            Window::close();
        }

        if (Input::getThumbStickLeftDistance() > 0.0f)
        {
            if (Input::isGamepadConnected())
            {
                Input::getConnectedController()->vibrate(0.5f, 0.5f, 100);
            }
        }

        if (Input::getThumbStickRightDistance() > 0.0f)
        {
            if (Input::isGamepadConnected())
            {
                Input::getConnectedController()->vibrate(1.0f, 1.0f, 100);
            }
        }

        if (Input::isKeyDown(KeyCode::ButtonNorth))
        {
            WS_LOG_INFO("Sandbox", "Button North pressed");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    Engine::shutdown();
}