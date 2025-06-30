#pragma once
#include "Types.hpp"

#include <cstdint>
#include <string>

namespace worse
{
    enum class WindowMode
    {
        Windowed,
        Fullscreen,
        FullscreenBorderless,
    };

    class Window : public NonCopyable, NonMovable
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
        static void setSize(std::uint32_t const w, std::uint32_t const h);
        static std::uint32_t getWidth();
        static std::uint32_t getHeight();
        static void setPosition(std::int32_t const x, std::int32_t const y);
        static std::pair<int, int> getPosition();

        static void* getHandleSDL();
        static void* getHandleNative();

    private:
        static inline std::uint32_t s_width  = 1200;
        static inline std::uint32_t s_height = 720;
        static inline std::string s_title    = "Window";
        static inline WindowMode s_mode      = WindowMode::Windowed;
        static inline bool s_shouldClose     = false;
    };

} // namespace worse