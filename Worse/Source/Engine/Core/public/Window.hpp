#pragma once
#include "base_type.hpp"

#include <string>

namespace Worse
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
        static Bool shouldClose();
        static Bool isMinimized();

        static void setWindowMode(WindowMode mode);
        static WindowMode getWindowMode();
        static void setSize(UInt const w, UInt const h);
        static void setPosition(Int const x, Int const y);
        static std::pair<Int, Int> getPosition();

        // clang-format off
        static UInt getWidth() { return s_width; }
        static UInt getHeight() { return s_height; }
        // clang-format on

        static void* getHandleSDL();
        static void* getHandleNative();

    private:
        static inline UInt s_width  = 1200;
        static inline UInt s_height = 800;

        static inline std::string s_title = "Window";
        static inline WindowMode s_mode   = WindowMode::Windowed;
        static inline Bool s_shouldClose  = false;
    };

} // namespace Worse