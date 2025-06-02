#include "Engine.hpp"
#include "Window.hpp"
#include "Input/Input.hpp"
#include "Input/Controller.hpp"

#include <thread>

using namespace worse;

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


        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    Engine::shutdown();
}