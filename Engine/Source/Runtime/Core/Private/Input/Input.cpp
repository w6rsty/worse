#include "Input/Input.hpp"
#include "Definitions.hpp"
#include "Window.hpp"

#include "SDL3/SDL_events.h"

namespace worse
{
    void Input::initialize()
    {
        WS_ASSERT_MSG(Window::getHandleSDL() != nullptr,
                      "Input system requires SDL to be initialized.");

        s_keymap.fill(false);
        s_keymapLastFrame.fill(false);

        EventBus::subscribe(EventType::SDL, Input::onEvent);
    }

    void Input::tick()
    {
        s_keymapLastFrame = s_keymap;

        pollMouse();
        pollKeyboard();
        pollGamepad();
    }

    Input::KeyMap& Input::GetKeyMap()
    {
        return s_keymap;
    }

    void Input::onEvent(Event const& payload)
    {
        SDL_Event* event = static_cast<SDL_Event*>(std::get<void*>(payload));

        onEventMouse(event);
        onEventGamepad(event);
    }

    bool Input::isKey(KeyCode const key)
    {
        return s_keymap[static_cast<std::size_t>(key)];
    }

    bool Input::isKeyDown(KeyCode const key)
    {
        return isKey(key) && !s_keymapLastFrame[static_cast<std::size_t>(key)];
    }

    bool Input::isKeyUp(KeyCode const key)
    {
        return !isKey(key) && s_keymapLastFrame[static_cast<std::size_t>(key)];
    }

} // namespace worse