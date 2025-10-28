#include "Definitions.hpp"
#include "Input/Input.hpp"
#include "Input/Controller.hpp"

#include "SDL3/SDL_events.h"

#include <cstdlib>

namespace worse
{
    namespace
    {
        std::shared_ptr<Controller> s_controller = nullptr;
        math::Vector2 s_thumbStickLeft{0.0f, 0.0f};
        math::Vector2 s_thumbStickRight{0.0f, 0.0f};
        f32 s_triggerLeft{0.0f};
        f32 s_triggerRight{0.0f};

        std::vector<ControllerDescriptor> fetchInfos()
        {
            std::vector<ControllerDescriptor> controllerDescriptors;
            int numGamepads             = 0;
            SDL_JoystickID* joystickIDs = SDL_GetGamepads(&numGamepads);
            controllerDescriptors.reserve(numGamepads);

            for (int i = 0; i < numGamepads; ++i)
            {
                SDL_Gamepad* gamepad = SDL_OpenGamepad(joystickIDs[i]);

                ControllerDescriptor cd = {};
                cd.joystickID           = joystickIDs[i];

                char guidStr[33];
                SDL_GUIDToString(SDL_GetGamepadGUIDForID(joystickIDs[i]),
                                 guidStr,
                                 sizeof(guidStr));

                cd.guid = std::string(guidStr);
                cd.name = SDL_GetGamepadName(gamepad);
                SDL_PowerState _powerState =
                    SDL_GetGamepadPowerInfo(gamepad, &cd.powerPercentage);
                switch (SDL_GetGamepadType(gamepad))
                {
                case SDL_GAMEPAD_TYPE_XBOX360:
                case SDL_GAMEPAD_TYPE_XBOXONE:
                    cd.type = ControllerType::Xbox;
                    break;
                case SDL_GAMEPAD_TYPE_PS3:
                case SDL_GAMEPAD_TYPE_PS4:
                case SDL_GAMEPAD_TYPE_PS5:
                    cd.type = ControllerType::PlayStation;
                    break;
                default:
                    cd.type = ControllerType::Common;
                    break;
                }

                SDL_GamepadConnected(gamepad);

                SDL_CloseGamepad(gamepad);

                controllerDescriptors.push_back(cd);
            }

            return controllerDescriptors;
        }

        std::optional<ControllerDescriptor> findAvailable()
        {
            auto controllerDescriptors = fetchInfos();
            if (controllerDescriptors.empty())
            {
                return std::nullopt;
            }

            // remove the currently connected controller if it exists
            if (s_controller)
            {
                auto it = std::remove_if(controllerDescriptors.begin(),
                                         controllerDescriptors.end(),
                                         [](ControllerDescriptor const& cd)
                                         {
                                             return *s_controller == cd;
                                         });
                controllerDescriptors.erase(it, controllerDescriptors.end());
            }

            if (controllerDescriptors.empty())
            {
                return std::nullopt;
            }

            return {controllerDescriptors[0]};
        }

        // TODO: Support individual dead zones for each axis
        f32 analogValue(SDL_Gamepad* gamepad, SDL_GamepadAxis const axis)
        {
            f32 normalized = 0.0f;

            static const i16 k_defaultThumbStickDeadZone = 8000;
            static const i16 k_defaultTriggerDeadZone    = 0;

            // For thumbsticks, value ranging from -32768 (up/left) to
            // 32767 (down/right). Triggers range from 0 when released
            // to 32767 when fully pressed, and never return a negative
            // value.

            switch (axis)
            {
            case SDL_GAMEPAD_AXIS_LEFTX:
            case SDL_GAMEPAD_AXIS_LEFTY:
            case SDL_GAMEPAD_AXIS_RIGHTX:
            case SDL_GAMEPAD_AXIS_RIGHTY:
            {
                i16 value = SDL_GetGamepadAxis(gamepad, axis);
                if (std::abs(value) < k_defaultThumbStickDeadZone)
                {
                    value = 0.0f;
                }
                else
                {
                    value -= (value > 0) ? k_defaultThumbStickDeadZone
                                         : -k_defaultThumbStickDeadZone;
                }

                f32 const range_negative = 32768.0f;
                f32 const range_positive = 32767.0f;
                f32 const range = (value < 0) ? range_negative : range_positive;
                normalized      = static_cast<f32>(value) /
                             (range - k_defaultThumbStickDeadZone);
                break;
            }
            case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
            case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
            {
                i16 value = SDL_GetGamepadAxis(gamepad, axis);
                if (value < k_defaultTriggerDeadZone)
                {
                    value = 0.0f;
                }
                else
                {
                    value -= k_defaultTriggerDeadZone;
                }
                f32 const range = 32767.0f - k_defaultTriggerDeadZone;
                normalized      = static_cast<f32>(value) / range;
                break;
            }
            default:
                WS_ASSERT_MSG(false, "Invalid axis");
                break;
            }

            return normalized;
        }
    } // namespace

    void Input::pollGamepad()
    {
        if (!s_controller)
        {
            return;
        }

        // clang-format off

        SDL_Gamepad* gamepad =
            static_cast<SDL_Gamepad*>(s_controller->getHandleSDL());

        // axis analog keys
        s_thumbStickLeft.x  = analogValue(gamepad, SDL_GAMEPAD_AXIS_LEFTX);
        s_thumbStickLeft.y  = analogValue(gamepad, SDL_GAMEPAD_AXIS_LEFTY);
        s_thumbStickRight.x = analogValue(gamepad, SDL_GAMEPAD_AXIS_RIGHTX);
        s_thumbStickRight.y = analogValue(gamepad, SDL_GAMEPAD_AXIS_RIGHTY);
        s_triggerLeft       = analogValue(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
        s_triggerRight      = analogValue(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);

        KeyMap& keyMap = GetKeyMap();
        keyMap[static_cast<usize>(KeyCode::DPadUp)]        = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP);
        keyMap[static_cast<usize>(KeyCode::DPadDown)]      = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
        keyMap[static_cast<usize>(KeyCode::DPadLeft)]      = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
        keyMap[static_cast<usize>(KeyCode::DPadRight)]     = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
        keyMap[static_cast<usize>(KeyCode::ButtonNorth)]   = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_NORTH);
        keyMap[static_cast<usize>(KeyCode::ButtonSouth)]   = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_SOUTH);
        keyMap[static_cast<usize>(KeyCode::ButtonWest)]    = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_WEST);
        keyMap[static_cast<usize>(KeyCode::ButtonEast)]    = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_EAST);
        keyMap[static_cast<usize>(KeyCode::Back)]          = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_BACK);
        keyMap[static_cast<usize>(KeyCode::Start)]         = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_START);
        keyMap[static_cast<usize>(KeyCode::Guide)]         = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_GUIDE);
        keyMap[static_cast<usize>(KeyCode::Touchpad)]      = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_TOUCHPAD);
        keyMap[static_cast<usize>(KeyCode::LeftStick)]     = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_LEFT_STICK);
        keyMap[static_cast<usize>(KeyCode::RightStick)]    = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_RIGHT_STICK);
        keyMap[static_cast<usize>(KeyCode::LeftShoulder)]  = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
        keyMap[static_cast<usize>(KeyCode::RightShoulder)] = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
        keyMap[static_cast<usize>(KeyCode::Misc)]          = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_MISC1);
        // clang-format on
    }

    void Input::onEventGamepad(void* event)
    {
        SDL_Event* sdlEvent = static_cast<SDL_Event*>(event);

        if ((sdlEvent->type == SDL_EVENT_JOYSTICK_ADDED) ||
            (sdlEvent->type == SDL_EVENT_GAMEPAD_ADDED))
        {
            if (!s_controller)
            {
                if (auto cd = findAvailable(); cd.has_value())
                {
                    s_controller = std::make_shared<Controller>(cd.value());
                }
            }
        }

        if ((sdlEvent->type == SDL_EVENT_JOYSTICK_REMOVED) ||
            (sdlEvent->type == SDL_EVENT_GAMEPAD_REMOVED))
        {
            if (s_controller)
            {
                if (s_controller->getJoystickID() == sdlEvent->gdevice.which)
                {
                    s_controller.reset();
                }
            }

            // try to find a new controller
            if (auto cd = findAvailable(); cd.has_value())
            {
                s_controller = std::make_shared<Controller>(cd.value());
            }
        }

        if (sdlEvent->type == SDL_EVENT_QUIT)
        {
            if (s_controller)
            {
                s_controller.reset();
            }
        }
    }

    bool Input::isGamepadConnected()
    {
        return (s_controller != nullptr) && s_controller->isConnected();
    }

    math::Vector2 const& Input::getThumbStickLeft()
    {
        return s_thumbStickLeft;
    }

    math::Vector2 const& Input::getThumbStickRight()
    {
        return s_thumbStickRight;
    }

    f32 Input::getThumbStickLeftDistance()
    {
        return length(s_thumbStickLeft);
    }

    f32 Input::getThumbStickRightDistance()
    {
        return length(s_thumbStickRight);
    }

    f32 Input::getTriggerLeft()
    {
        return s_triggerLeft;
    }

    f32 Input::getTriggerRight()
    {
        return s_triggerRight;
    }

    Controller* Input::getConnectedController()
    {
        return s_controller.get();
    }

} // namespace worse