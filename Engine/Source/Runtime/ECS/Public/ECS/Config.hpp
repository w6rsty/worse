#pragma once
#include <bit>
#include <cstddef>
#include <cassert>
#include <type_traits>

namespace worse::ecs
{
    constexpr std::size_t SPARSE_PAGE_SIZE = 4096UL;
    constexpr std::size_t PACKED_PAGE_SIZE = 1024UL;

    template <typename Type>
        requires(std::is_unsigned_v<Type>)
    constexpr Type fast_mod(Type const value, std::size_t const mod) noexcept
    {
        assert(std::has_single_bit(mod) && "mod must be power of 2");
        return static_cast<Type>(value & (mod - 1u));
    }
}; // namespace worse::ecs