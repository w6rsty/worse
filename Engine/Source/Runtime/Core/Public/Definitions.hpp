#pragma once
#include "Log.hpp"

namespace worse
{

#ifdef DEBUG

#include <cassert>
#define WS_ASSERT(expression)                                                  \
    if (!(expression))                                                         \
    {                                                                          \
        WS_LOG_ERROR("Assertion failed", #expression);                         \
        ::worse::Logger::instance()->waitShutdown();                           \
        assert(expression);                                                    \
    }

#define WS_ASSERT_MSG(expression, message)                                     \
    if (!(expression))                                                         \
    {                                                                          \
        WS_LOG_ERROR("Assertion failed", #expression);                         \
        WS_LOG_ERROR("Message", message);                                      \
        ::worse::Logger::instance()->waitShutdown();                           \
        assert(expression && message);                                         \
    }

#else

#define WS_ASSERT(expression)                                                  \
    do                                                                         \
    {                                                                          \
        (void)(expression);                                                    \
    } while (false)
#define WS_ASSERT_MSG(expression, message)                                     \
    do                                                                         \
    {                                                                          \
        (void)(expression);                                                    \
        (void)(message);                                                       \
    } while (false)

#endif

#define UNIMPLEMENTED() WS_ASSERT_MSG(false, "Unimplemented code")

} // namespace worse