#pragma once
#include "base_type.hpp"

namespace Worse
{

    template <Int IndexSize>
    struct BitsToSizeType
    {
    };

    // clang-format off
    template <> struct BitsToSizeType<8>  { using Type = Byte; };
    template <> struct BitsToSizeType<16> { using Type = Short; };
    template <> struct BitsToSizeType<32> { using Type = Int; };
    template <> struct BitsToSizeType<64> { using Type = Long; };
    // clang-format on

    template <Int IndexSize>
    class SizedHeapAllocator
    {
    public:
        using SizeType = typename BitsToSizeType<IndexSize>::Type;

        enum
        {
            NeedsElementType = kFalse
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

} // namespace Worse
