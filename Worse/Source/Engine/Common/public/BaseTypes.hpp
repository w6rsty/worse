#pragma once

#include <cstdint>
#include <cstddef>

namespace worse
{

    using I8  = std::int8_t;
    using U8  = std::uint8_t;
    using I16 = std::int16_t;
    using U16 = std::uint16_t;
    using I32 = std::int32_t;
    using U32 = std::uint32_t;
    using I64 = std::int64_t;
    using U64 = std::uint64_t;

    using Size = std::size_t;

    using DPtr = std::ptrdiff_t;
    using IPtr = std::intptr_t;
    using UPtr = std::uintptr_t;

    using F32 = float;
    using F64 = double;

    using Byte = std::byte;

    using Char  = char;
    using UChar = unsigned char;

} // namespace worse