#pragma once
#include "Types.hpp"

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
        static void setSize(u32 const w, u32 const h);
        static u32 getWidth();
        static u32 getHeight();
        static void setPosition(i32 const x, i32 const y);
        static std::pair<int, int> getPosition();

        static void* getHandleSDL();
        static void* getHandleNative();

    private:
        static inline u32 s_width         = 1200;
        static inline u32 s_height        = 720;
        static inline std::string s_title = "Window";
        static inline WindowMode s_mode   = WindowMode::Windowed;
        static inline bool s_shouldClose  = false;
    };

} // namespace worse