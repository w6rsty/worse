#include "Window.hpp"
#include "Log.hpp"
#include "Event.hpp"
#include "Definitions.hpp"

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"

namespace worse
{

    namespace
    {
        SDL_Window* s_window = nullptr;
    } // namespace

    void Window::initialize()
    {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
        {
            WS_LOG_FATAL("Window",
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
            WS_LOG_FATAL("Window",
                         "Failed to create window: {}",
                         SDL_GetError());
        }
    }

    void Window::shutdown()
    {
        WS_ASSERT(s_window);

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
                s_width  = static_cast<std::uint32_t>(event.window.data1);
                s_height = static_cast<std::uint32_t>(event.window.data2);

                EventBus::fire(EventType::WindowResized);
                break;
            }
            default:
                // Ohter events are delegated to the EventBus
                break;
            }

            EventBus::fire(EventType::SDL, &event);
        }
    }

    void Window::show()
    {
        WS_ASSERT(s_window);

        SDL_ShowWindow(s_window);
    }

    void Window::hide()
    {
        WS_ASSERT(s_window);

        SDL_HideWindow(s_window);
    }

    void Window::close()
    {
        s_shouldClose = true;
    }

    bool Window::shouldClose()
    {
        return s_shouldClose;
    }

    bool Window::isMinimized()
    {
        return SDL_GetWindowFlags(s_window) & SDL_WINDOW_MINIMIZED;
    }

    void Window::setWindowMode(WindowMode mode)
    {
        WS_ASSERT(s_window);

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

    void Window::setSize(std::uint32_t const w, std::uint32_t const h)
    {
        WS_ASSERT(s_window);
        SDL_SetWindowSize(s_window, static_cast<int>(w), static_cast<int>(h));
        s_width  = w;
        s_height = h;
    }

    std::uint32_t Window::getWidth()
    {
        return s_width;
    }

    std::uint32_t Window::getHeight()
    {
        return s_height;
    }

    void Window::setPosition(std::int32_t const x, std::int32_t const y)
    {
        SDL_SetWindowPosition(s_window,
                              static_cast<int>(x),
                              static_cast<int>(y));
    }

    std::pair<int, int> Window::getPosition()
    {
        WS_ASSERT(s_window);
        int x = 0;
        int y = 0;
        SDL_GetWindowPosition(s_window, &x, &y);
        return {x, y};
    }

    // std::optional<ControllerDescriptor> Window::findAvailableController()
    // {
    //     auto controllerDescriptors = controller::fetchInfos();
    //     if (controllerDescriptors.empty())
    //     {
    //         return std::nullopt;
    //     }

    //     // remove the currently connected controller if it exists
    //     if (isControllerConnected())
    //     {
    //         auto it = std::remove_if(controllerDescriptors.begin(),
    //                                  controllerDescriptors.end(),
    //                                  [](ControllerDescriptor const& cd)
    //                                  {
    //                                      return *controller::s_controller ==
    //                                      cd;
    //                                  });
    //         controllerDescriptors.erase(it, controllerDescriptors.end());
    //     }

    //     if (controllerDescriptors.empty())
    //     {
    //         return std::nullopt;
    //     }

    //     return {controllerDescriptors[0]};
    // }

    // bool Window::isControllerConnected()
    // {
    //     return controller::s_controller != nullptr;
    // }

    // Controller* Window::getConnectedController()
    // {
    //     return controller::s_controller.get();
    // }

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

} // namespace worse