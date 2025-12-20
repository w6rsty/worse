#pragma once

#include "BaseTypes.hpp"

namespace worse
{

    template <I32 IndexType>
    class SizedDefaultAllocator;
    using DefaultAllocator = SizedDefaultAllocator<32>;

    template <typename T, typename Allocator = DefaultAllocator>
    class Array;

}; // namespace worse
