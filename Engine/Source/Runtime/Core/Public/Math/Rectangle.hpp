#pragma once
#include <cstdint>

namespace worse::math
{

    struct Rectangle
    {
        std::int32_t x       = 0;
        std::int32_t y       = 0;
        std::uint32_t width  = 0;
        std::uint32_t height = 0;
    };

} // namespace worse::math