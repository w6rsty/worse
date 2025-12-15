#pragma once
#include "BaseTypes.hpp"

#include <string>

namespace worse
{
    enum class WindowMode
    {
        Windowed,
        Fullscreen,
        FullscreenBorderless,
    };

    class Window
    {
    public:
        static void initialize();
        static void shutdown();
        static void tick();

        static void show();
        static void hide();
        static void close();
        static bool shouldClose();
        static bool isMinimized();

        static void setWindowMode(WindowMode mode);
        static WindowMode getWindowMode();
        static void setSize(U32 const w, U32 const h);
        static void setPosition(I32 const x, I32 const y);
        static std::pair<I32, I32> getPosition();

        // clang-format off
        static U32 getWidth() { return s_width; }
        static U32 getHeight() { return s_height; }
        // clang-format on

        static void* getHandleSDL();
        static void* getHandleNative();

    private:
        static inline U32 s_width  = 1200;
        static inline U32 s_height = 800;

        static inline std::string s_title = "Window";
        static inline WindowMode s_mode   = WindowMode::Windowed;
        static inline bool s_shouldClose  = false;
    };

} // namespace worse