#pragma once
#include <cstdlib>


///////////////
// assertion //
///////////////
#define WORSE_ASSERT(x)   \
    do                    \
    {                     \
        if (!(x))         \
        {                 \
            std::abort(); \
        }                 \
    } while (0)

#define WORSE_ASSERT_MSG(x, msg) WORSE_ASSERT(x)

#define WORSE_STATIC_ASSERT(x, msg) static_assert(x, msg)

/////////////////
// debug break //
/////////////////
// clang-format off
#if defined(_MSC_VER)
    #include <intrin.h>

    #define WORSE_DEBUG_BREAK() __debugbreak()
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
    #include <csignal>

    #if defined(__has_builtin)
        #if __has_builtin(__builtin_debugtrap)
            #define WORSE_DEBUG_BREAK() __builtin_debugtrap()
        #elif __has_builtin(__builtin_trap)
            #define WORSE_DEBUG_BREAK() __builtin_trap()
        #else
            #define WORSE_DEBUG_BREAK() raise(SIGTRAP)
        #endif
    #else
        #define WORSE_DEBUG_BREAK() raise(SIGTRAP)
    #endif
#else
    #error "Unsupported platform for DEBUG_BREAK"
#endif
// clang-format on

/////////////////
// unreachable //
/////////////////
// clang-format off
#if defined(_MSC_VER)
    #define WORSE_UNREACHABLE_IMPL() __assume(0)
#elif defined(__clang__) || defined(__GNUC__)
    #define WORSE_UNREACHABLE_IMPL() __builtin_unreachable()
#else
    #define WORSE_UNREACHABLE_IMPL() ((void)0)
#endif

#if defined(NDEBUG)
    #define WORSE_UNREACHABLE() WORSE_UNREACHABLE_IMPL()
#else
    #define WORSE_UNREACHABLE()                                       \
        do                                                            \
        {                                                             \
            std::cerr << "UNREACHABLE reached at " << __FILE__ << ":" \
                      << __LINE__ << std::endl;                       \
            DEBUG_BREAK();                                            \
            std::abort();                                             \
        } while (0)
#endif
// clang-format on

//////////////////
// force inline //
//////////////////
// clang-format off
#if defined(_MSC_VER)
    #define WORSE_FORCE_INLINE __forceinline
#elif defined(__clang__) || defined(__GNUC__)
    #define WORSE_FORCE_INLINE inline __attribute__((always_inline))
#else
    #define WORSE_FORCE_INLINE inline
#endif
// clang-format on

#define WORSE_UNIMPLEMENTED() WORSE_ASSERT_MSG(false, "Unimplemented code")

//////////
// cast //
//////////
#define s_cast static_cast
#define r_cast reinterpret_cast
#define c_cast const_cast
#define d_cast dynamic_cast