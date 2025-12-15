#pragma once

#include "BaseTypes.hpp"

namespace worse
{

    template <I32 IndexSize>
    struct BitsToSizeType
    {
    };

    // clang-format off
    template <> struct BitsToSizeType<8>  { using Type = I8; };
    template <> struct BitsToSizeType<16> { using Type = I16; };
    template <> struct BitsToSizeType<32> { using Type = I32; };
    template <> struct BitsToSizeType<64> { using Type = I64; };
    // clang-format on

    template <I32 IndexSize>
    class SizedHeapAllocator
    {
    public:
        using SizeType = typename BitsToSizeType<IndexSize>::Type;

        enum
        {
            NeedsElementType = false
        };

        class ForAnyElementType
        {
        public:
            SizeType GetInitialCapacity() const
            {
                return 0;
            }
        };

        template <typename ElementType>
        class ForElementType : public ForAnyElementType
        {
        public:
        };
    };

} // namespace worse
