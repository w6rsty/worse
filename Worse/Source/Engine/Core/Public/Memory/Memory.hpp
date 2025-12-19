#pragma once

#include "BaseTypes.hpp"

namespace worse
{
    constexpr U32 kDefaultAlignment = 0;

    struct Memory
    {
        static void* Malloc(Size count, U32 alignment = kDefaultAlignment);
        static void* Realloc(void* original, Size count, U32 alignment = kDefaultAlignment);
        static void Free(void* original);
    };

} // namespace worse
