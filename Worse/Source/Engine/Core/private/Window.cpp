#include "common_macro.hpp"
#include "logger/logger.hpp"
#include "Window.hpp"
#include "Event.hpp"

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"

namespace Worse
{

    namespace
    {
        SDL_Window* s_window = nullptr;
    } // namespace

    void Window::initialize()
    {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
        {
            WORSE_LOG_ERROR("Window",
                            "Failed to initialize SDL: {}",
                            SDL_GetError());
        }

        SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;
        s_window              = SDL_CreateWindow(s_title.c_str(),
                                    static_cast<int>(s_width),
                                    static_cast<int>(s_height),
                                    flags);

        if (!s_window)
        {
            WORSE_LOG_ERROR("Window",
                            "Failed to create window: {}",
                            SDL_GetError());
        }
    }

    void Window::shutdown()
    {
        WORSE_ASSERT(s_window);

        SDL_DestroyWindow(s_window);
        s_window = nullptr;

        SDL_Quit();
    }

    void Window::tick()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                s_shouldClose = true;
                break;
            case SDL_EVENT_WINDOW_RESIZED:
            {
                s_width  = static_cast<UInt>(event.window.data1);
                s_height = static_cast<UInt>(event.window.data2);

                EventBus::fire(EventType::WindowResized);
                break;
            }
            default:
                // Other events are delegated to the EventBus
                break;
            }

            EventBus::fire(EventType::SDL, &event);
        }
    }

    void Window::show()
    {
        WORSE_ASSERT(s_window);

        SDL_ShowWindow(s_window);
    }

    void Window::hide()
    {
        WORSE_ASSERT(s_window);

        SDL_HideWindow(s_window);
    }

    void Window::close()
    {
        s_shouldClose = true;
    }

    Bool Window::shouldClose()
    {
        return s_shouldClose;
    }

    Bool Window::isMinimized()
    {
        return SDL_GetWindowFlags(s_window) & SDL_WINDOW_MINIMIZED;
    }

    void Window::setWindowMode(WindowMode mode)
    {
        WORSE_ASSERT(s_window);

        switch (mode)
        {
        case WindowMode::Windowed:
            SDL_SetWindowFullscreen(s_window, 0);
            break;
        case WindowMode::Fullscreen:
        {
            SDL_Rect rect{};
            SDL_GetDisplayUsableBounds(SDL_GetPrimaryDisplay(), &rect);
            Window::setPosition(0, 0);
            Window::setSize(rect.w, rect.h);
            break;
        }
        case WindowMode::FullscreenBorderless:
            SDL_SetWindowFullscreen(s_window, true);
            break;
        }

        s_mode = mode;
    }

    WindowMode Window::getWindowMode()
    {
        return s_mode;
    }

    void Window::setSize(UInt const w, UInt const h)
    {
        WORSE_ASSERT(s_window);
        SDL_SetWindowSize(s_window, static_cast<int>(w), static_cast<int>(h));
        s_width  = w;
        s_height = h;
    }

    void Window::setPosition(Int const x, Int const y)
    {
        SDL_SetWindowPosition(s_window,
                              static_cast<Int>(x),
                              static_cast<Int>(y));
    }

    std::pair<Int, Int> Window::getPosition()
    {
        WORSE_ASSERT(s_window);
        Int x = 0;
        Int y = 0;
        SDL_GetWindowPosition(s_window, &x, &y);
        return {x, y};
    }

    void* Window::getHandleSDL()
    {
        return static_cast<void*>(s_window);
    }

    void* Window::getHandleNative()
    {
        SDL_PropertiesID prop = SDL_GetWindowProperties(s_window);

        // MacOS
        if (void* handle =
                SDL_GetPointerProperty(prop,
                                       SDL_PROP_WINDOW_COCOA_WINDOW_POINTER,
                                       nullptr))
        {
            return handle;
        }

        return nullptr;
    }

} // namespace Worse