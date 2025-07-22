#pragma once
#include <bit>
#include <cstddef>
#include <cassert>
#include <type_traits>

namespace worse::ecs
{
    constexpr usize SPARSE_PAGE_SIZE = 4096UL;
    constexpr usize PACKED_PAGE_SIZE = 1024UL;

    template <typename Type>
        requires(std::is_unsigned_v<Type>)
    constexpr Type fast_mod(Type const value, usize const mod) noexcept
    {
        assert(std::has_single_bit(mod) && "mod must be power of 2");
        return static_cast<Type>(value & (mod - 1u));
    }
}; // namespace worse::ecs