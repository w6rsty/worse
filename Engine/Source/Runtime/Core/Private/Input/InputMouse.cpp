#include "Input/Input.hpp"
#include "Log.hpp"
#include "Window.hpp"

#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_events.h"

#include <cstddef>

namespace worse
{

    namespace
    {
        Vector2 s_mousePosition{0.0f, 0.0f};
        Vector2 s_mouseDelta{0.0f, 0.0f};
        Vector2 s_mouseWheelDelta{0.0f, 0.0f};
    } // namespace

    void Input::pollMouse()
    {
        float x{0.0f}, y{0.0f};
        SDL_MouseButtonFlags mouseStates = SDL_GetGlobalMouseState(&x, &y);
        Vector2 position{x, y};

        s_mouseDelta    = position - s_mousePosition;
        s_mousePosition = position;

        KeyMap& keymap = Input::GetKeyMap();
        // clang-format off
        keymap[static_cast<std::size_t>(KeyCode::ClickLeft)]   = (mouseStates & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) != 0;
        keymap[static_cast<std::size_t>(KeyCode::ClickMiddle)] = (mouseStates & SDL_BUTTON_MASK(SDL_BUTTON_MIDDLE)) != 0;
        keymap[static_cast<std::size_t>(KeyCode::ClickRight)]  = (mouseStates & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) != 0;
        // clang-format on
    }

    void Input::onEventMouse(void* event)
    {
        SDL_Event* sdlEvent     = static_cast<SDL_Event*>(event);
        std::uint32_t eventType = sdlEvent->type;

        if (eventType == SDL_EVENT_MOUSE_WHEEL)
        {
            // clang-format off
            if (sdlEvent->wheel.x > 0.0f) s_mouseWheelDelta.x += 1.0f;
            if (sdlEvent->wheel.x < 0.0f) s_mouseWheelDelta.x -= 1.0f;
            if (sdlEvent->wheel.y > 0.0f) s_mouseWheelDelta.y += 1.0f;
            if (sdlEvent->wheel.y < 0.0f) s_mouseWheelDelta.y -= 1.0f;
            // clang-format on
        }
    }

    void Input::setMouseVisible(bool const visible)
    {
        if (visible)
        {
            SDL_ShowCursor();
        }
        else
        {
            SDL_HideCursor();
        }
    }

    bool Input::getMouseVisible()
    {
        return SDL_CursorVisible();
    }

    void Input::setMousePositionGlobal(Vector2 const& position)
    {
        if (!SDL_WarpMouseGlobal(position.x, position.y))
        {
            WS_LOG_ERROR("Input", "Failed to set mouse position");
            return;
        }
        s_mousePosition = position;
    }

    Vector2 const& Input::getMouseDelta()
    {
        return s_mouseDelta;
    }

    Vector2 const& Input::getMouseWheelDelta()
    {
        return s_mouseWheelDelta;
    }

    Vector2 const& Input::getMousePositionGlobal()
    {
        return s_mousePosition;
    }

    Vector2 Input::getMousePositionRelativeToWindow()
    {
        auto [windowX, windowY] = Window::getPosition();
        return Vector2{static_cast<float>(s_mousePosition.x - windowX),
                       static_cast<float>(s_mousePosition.y - windowY)};
    }

    Vector2 Input::getMousePositionRelativeToViewport()
    {
        // TODO: Calculate position relative to the viewport
        return Input::getMousePositionRelativeToWindow();
    }

} // namespace worse