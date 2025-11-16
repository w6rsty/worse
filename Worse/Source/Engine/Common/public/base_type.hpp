#pragma once
#include <cstddef>
#include <cstdint>

namespace Worse
{

    using Bool = bool;

    using Char  = char;
    using UChar = unsigned char;

    using Byte   = std::int8_t;
    using UByte  = std::uint8_t;
    using Short  = std::int16_t;
    using UShort = std::uint16_t;
    using Int    = std::int32_t;
    using UInt   = std::uint32_t;
    using Long   = std::int64_t;
    using ULong  = std::uint64_t;

    using Size    = std::size_t;
    using PtrDiff = std::ptrdiff_t;
    using PtrInt  = std::intptr_t;
    using UPtrInt = std::uintptr_t;

    using Float  = float;
    using Double = double;

    constexpr Bool kTrue  = true;
    constexpr Bool kFalse = false;

} // namespace Worse