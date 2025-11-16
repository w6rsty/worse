#pragma once
#include "base_type.hpp"

namespace Worse
{
    constexpr UInt kDefaultAlignment = 0;

    struct Memory
    {
        static void* Malloc(Size count, UInt alignment = kDefaultAlignment);
        static void* Realloc(void* original, Size count, UInt alignment = kDefaultAlignment);
        static void Free(void* original);
    };

} // namespace Worse
