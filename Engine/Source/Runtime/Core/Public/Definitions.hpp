#pragma once
#include "Log.hpp"

namespace worse
{

#include <cassert>
#define WS_ASSERT(expression)                                                  \
    if (!(expression))                                                         \
    {                                                                          \
        WS_LOG_ERROR("Assertion failed", #expression);                         \
        assert(expression);                                                    \
    }

#define WS_ASSERT_MSG(expression, message)                                     \
    if (!(expression))                                                         \
    {                                                                          \
        WS_LOG_ERROR("Assertion failed", #expression);                         \
        WS_LOG_ERROR("Message", message);                                      \
        assert(expression && message);                                         \
    }

} // namespace worse