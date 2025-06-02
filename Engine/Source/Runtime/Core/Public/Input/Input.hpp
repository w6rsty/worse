#pragma once
#include "Types.hpp"
#include "Event.hpp"
#include "Math/Vector2.hpp"

#include <array>

namespace worse
{
    class Controller;

    enum class KeyCode
    {
        // clang-format off

        // keyboard
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
        Alpha0, Alpha1, Alpha2, Alpha3, Alpha4, Alpha5, Alpha6, Alpha7, Alpha8, Alpha9,
        Q, W, E, R, T, Y, U, I, O, P,
        A, S, D, F, G, H, J, K, L,
        Z, X, C, V, B, N, M,
        ShiftLeft, ShiftRight,
        CtrlLeft, CtrlRight,
        AltLeft, AltRight,
        Escape, Tab, Capslock, Backspace, Enter, Space,
        ArrowLeft, ArrowRight, ArrowUp, ArrowDown,

        // mouse
        ClickLeft, ClickMiddle, ClickRight,

        // gamepad
        DPadUp, DPadDown, DPadLeft, DPadRight,
        ButtonNorth, ButtonSouth, ButtonWest, ButtonEast,
        Back,
        Start,
        Guide, // logo button
        Touchpad, // ps touchpad
        LeftStick, RightStick,
        LeftShoulder, RightShoulder,
        Misc, // xbox share button, ps5 microphone button

        Max
        // clang-format on
    };
    static constexpr std::size_t k_keyCodeCount =
        static_cast<std::size_t>(KeyCode::Max);

    class Input : public NonCopyable, public NonMovable
    {
        static void pollKeyboard();
        static void pollMouse();
        static void pollGamepad();

        static void onEvent(Event const& payload);
        static void onEventMouse(void* event);
        static void onEventGamepad(void* event);

        using KeyMap = std::array<bool, k_keyCodeCount>;
        static KeyMap& GetKeyMap();

    public:
        static void initialize();
        static void tick();

        // Keyboard
        // up, down, pressed
        static bool isKey(KeyCode const key);
        // down edge
        static bool isKeyDown(KeyCode const key);
        // up edge
        static bool isKeyUp(KeyCode const key);

        // Mouse
        static void setMouseVisible(bool const visible);
        static bool getMouseVisible();
        static math::Vector2 const& getMouseDelta();
        static math::Vector2 const& getMouseWheelDelta();
        // System position
        static void setMousePositionGlobal(math::Vector2 const& position);
        static math::Vector2 const& getMousePositionGlobal();
        static math::Vector2 getMousePositionRelativeToWindow();
        static math::Vector2 getMousePositionRelativeToViewport();

        // Gamepad
        static bool isGamepadConnected();
        // safe to call even if no controller is connected
        static math::Vector2 const& getThumbStickLeft();
        static math::Vector2 const& getThumbStickRight();
        static float getThumbStickLeftDistance();
        static float getThumbStickRightDistance();
        static float getTriggerLeft();
        static float getTriggerRight();
        // must validate that a controller is connected before deferencing
        static Controller* getConnectedController();

    private:
        static inline Input::KeyMap s_keymap{};
        static inline Input::KeyMap s_keymapLastFrame{};
    };

} // namespace worse