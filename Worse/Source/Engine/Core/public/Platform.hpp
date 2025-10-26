#pragma once

namespace worse
{
    enum class Platform
    {
        Unknown,
        Windows,
        Apple
    };

#if defined(WS_PLATFORM_WINDOWS)
    constexpr Platform CurrentPlatform = Platform::Windows;
#elif defined(WS_PLATFORM_APPLE)
    constexpr Platform CurrentPlatform = Platform::Apple;
#else
    constexpr Platform CurrentPlatform = Platform::Unknown;
#endif

#if defined(WS_ENGINE_DIR)
    constexpr char const* EngineDirectory = WS_ENGINE_DIR;
#else
    constexpr char const* EngineDirectory = "./";
#endif

}