#include "Log.hpp"
#include "Engine.hpp"
#include "Window.hpp"
#include "Input/Input.hpp"
#include "Input/Controller.hpp"

using namespace worse;

int main()
{
    bool mouseVisible = true;

    Logger::initialize();
    Engine::initialize();

    while (!Window::shouldClose())
    {
        Engine::tick();

        if (Input::isKeyDown(KeyCode::Escape))
        {
            Window::close();
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
    }

    Engine::shutdown();
    Logger::shutdown();
}
