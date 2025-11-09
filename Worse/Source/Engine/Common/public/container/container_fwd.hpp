#pragma once
#include "base_type.hpp"

namespace Worse
{

    template <Int IndexType>
    class SizedDefaultAllocator;
    using DefaultAllocator = SizedDefaultAllocator<32>;

    template <typename T, typename Allocator = DefaultAllocator>
    class Array;

}; // namespace Worse
