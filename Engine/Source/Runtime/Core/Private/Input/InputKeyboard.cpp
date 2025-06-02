#include "Definitions.hpp"
#include "Input/Input.hpp"

#include "SDL3/SDL_keyboard.h"
#include "SDL3/SDL_scancode.h"

namespace worse
{
    void Input::pollKeyboard()
    {
        int numKeys{0};
        bool const* keyStates = SDL_GetKeyboardState(&numKeys);

        WS_ASSERT_MSG(keyStates != nullptr, "Failed to get keyboard state");

        KeyMap& keyMap = GetKeyMap();
        // clang-format off
        keyMap[static_cast<std::size_t>(KeyCode::F1)]  = keyStates[SDL_SCANCODE_F1];
        keyMap[static_cast<std::size_t>(KeyCode::F2)]  = keyStates[SDL_SCANCODE_F2];
        keyMap[static_cast<std::size_t>(KeyCode::F3)]  = keyStates[SDL_SCANCODE_F3];
        keyMap[static_cast<std::size_t>(KeyCode::F4)]  = keyStates[SDL_SCANCODE_F4];
        keyMap[static_cast<std::size_t>(KeyCode::F5)]  = keyStates[SDL_SCANCODE_F5];
        keyMap[static_cast<std::size_t>(KeyCode::F6)]  = keyStates[SDL_SCANCODE_F6];
        keyMap[static_cast<std::size_t>(KeyCode::F7)]  = keyStates[SDL_SCANCODE_F7];
        keyMap[static_cast<std::size_t>(KeyCode::F8)]  = keyStates[SDL_SCANCODE_F8];
        keyMap[static_cast<std::size_t>(KeyCode::F9)]  = keyStates[SDL_SCANCODE_F9];
        keyMap[static_cast<std::size_t>(KeyCode::F10)] = keyStates[SDL_SCANCODE_F10];
        keyMap[static_cast<std::size_t>(KeyCode::F11)] = keyStates[SDL_SCANCODE_F11];
        keyMap[static_cast<std::size_t>(KeyCode::F12)] = keyStates[SDL_SCANCODE_F12];

        keyMap[static_cast<std::size_t>(KeyCode::Alpha0)] = keyStates[SDL_SCANCODE_0];
        keyMap[static_cast<std::size_t>(KeyCode::Alpha1)] = keyStates[SDL_SCANCODE_1];
        keyMap[static_cast<std::size_t>(KeyCode::Alpha2)] = keyStates[SDL_SCANCODE_2];
        keyMap[static_cast<std::size_t>(KeyCode::Alpha3)] = keyStates[SDL_SCANCODE_3];
        keyMap[static_cast<std::size_t>(KeyCode::Alpha4)] = keyStates[SDL_SCANCODE_4];
        keyMap[static_cast<std::size_t>(KeyCode::Alpha5)] = keyStates[SDL_SCANCODE_5];
        keyMap[static_cast<std::size_t>(KeyCode::Alpha6)] = keyStates[SDL_SCANCODE_6];
        keyMap[static_cast<std::size_t>(KeyCode::Alpha7)] = keyStates[SDL_SCANCODE_7];
        keyMap[static_cast<std::size_t>(KeyCode::Alpha8)] = keyStates[SDL_SCANCODE_8];
        keyMap[static_cast<std::size_t>(KeyCode::Alpha9)] = keyStates[SDL_SCANCODE_9];

        keyMap[static_cast<std::size_t>(KeyCode::A)] = keyStates[SDL_SCANCODE_A];
        keyMap[static_cast<std::size_t>(KeyCode::B)] = keyStates[SDL_SCANCODE_B];
        keyMap[static_cast<std::size_t>(KeyCode::Q)] = keyStates[SDL_SCANCODE_Q];
        keyMap[static_cast<std::size_t>(KeyCode::C)] = keyStates[SDL_SCANCODE_C];
        keyMap[static_cast<std::size_t>(KeyCode::D)] = keyStates[SDL_SCANCODE_D];
        keyMap[static_cast<std::size_t>(KeyCode::E)] = keyStates[SDL_SCANCODE_E];
        keyMap[static_cast<std::size_t>(KeyCode::F)] = keyStates[SDL_SCANCODE_F];
        keyMap[static_cast<std::size_t>(KeyCode::G)] = keyStates[SDL_SCANCODE_G];
        keyMap[static_cast<std::size_t>(KeyCode::H)] = keyStates[SDL_SCANCODE_H];
        keyMap[static_cast<std::size_t>(KeyCode::I)] = keyStates[SDL_SCANCODE_I];
        keyMap[static_cast<std::size_t>(KeyCode::J)] = keyStates[SDL_SCANCODE_J];
        keyMap[static_cast<std::size_t>(KeyCode::K)] = keyStates[SDL_SCANCODE_K];
        keyMap[static_cast<std::size_t>(KeyCode::L)] = keyStates[SDL_SCANCODE_L];
        keyMap[static_cast<std::size_t>(KeyCode::M)] = keyStates[SDL_SCANCODE_M];
        keyMap[static_cast<std::size_t>(KeyCode::N)] = keyStates[SDL_SCANCODE_N];
        keyMap[static_cast<std::size_t>(KeyCode::O)] = keyStates[SDL_SCANCODE_O];
        keyMap[static_cast<std::size_t>(KeyCode::P)] = keyStates[SDL_SCANCODE_P];
        keyMap[static_cast<std::size_t>(KeyCode::R)] = keyStates[SDL_SCANCODE_R];
        keyMap[static_cast<std::size_t>(KeyCode::S)] = keyStates[SDL_SCANCODE_S];
        keyMap[static_cast<std::size_t>(KeyCode::T)] = keyStates[SDL_SCANCODE_T];
        keyMap[static_cast<std::size_t>(KeyCode::U)] = keyStates[SDL_SCANCODE_U];
        keyMap[static_cast<std::size_t>(KeyCode::V)] = keyStates[SDL_SCANCODE_V];
        keyMap[static_cast<std::size_t>(KeyCode::W)] = keyStates[SDL_SCANCODE_W];
        keyMap[static_cast<std::size_t>(KeyCode::X)] = keyStates[SDL_SCANCODE_X];
        keyMap[static_cast<std::size_t>(KeyCode::Y)] = keyStates[SDL_SCANCODE_Y];
        keyMap[static_cast<std::size_t>(KeyCode::Z)] = keyStates[SDL_SCANCODE_Z];

        keyMap[static_cast<std::size_t>(KeyCode::ShiftLeft)]  = keyStates[SDL_SCANCODE_LSHIFT];
        keyMap[static_cast<std::size_t>(KeyCode::ShiftRight)] = keyStates[SDL_SCANCODE_RSHIFT];
        keyMap[static_cast<std::size_t>(KeyCode::CtrlLeft)]   = keyStates[SDL_SCANCODE_LCTRL];
        keyMap[static_cast<std::size_t>(KeyCode::CtrlRight)]  = keyStates[SDL_SCANCODE_RCTRL];
        keyMap[static_cast<std::size_t>(KeyCode::AltLeft)]    = keyStates[SDL_SCANCODE_LALT];
        keyMap[static_cast<std::size_t>(KeyCode::AltRight)]   = keyStates[SDL_SCANCODE_RALT];
        keyMap[static_cast<std::size_t>(KeyCode::Escape)]        = keyStates[SDL_SCANCODE_ESCAPE];
        keyMap[static_cast<std::size_t>(KeyCode::Tab)]        = keyStates[SDL_SCANCODE_TAB];
        keyMap[static_cast<std::size_t>(KeyCode::Capslock)]   = keyStates[SDL_SCANCODE_CAPSLOCK];
        keyMap[static_cast<std::size_t>(KeyCode::Backspace)]  = keyStates[SDL_SCANCODE_BACKSPACE];
        keyMap[static_cast<std::size_t>(KeyCode::Enter)]      = keyStates[SDL_SCANCODE_RETURN];
        keyMap[static_cast<std::size_t>(KeyCode::Space)]      = keyStates[SDL_SCANCODE_SPACE];
        keyMap[static_cast<std::size_t>(KeyCode::ArrowLeft)]  = keyStates[SDL_SCANCODE_LEFT];
        keyMap[static_cast<std::size_t>(KeyCode::ArrowRight)] = keyStates[SDL_SCANCODE_RIGHT];
        keyMap[static_cast<std::size_t>(KeyCode::ArrowUp)]    = keyStates[SDL_SCANCODE_UP];
        keyMap[static_cast<std::size_t>(KeyCode::ArrowDown)]  = keyStates[SDL_SCANCODE_DOWN];
        // clang-format on
    }

} // namespace worse