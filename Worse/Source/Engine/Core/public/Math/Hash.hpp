#pragma once
#include "Types.hpp"
#include <cstdint>
#include <concepts>

namespace worse::math
{

    template <typename T>
    concept Hashable = requires(T a) {
        { a.hash() } -> std::convertible_to<u64>;
    } || requires(T a) {
        { hash(a) } -> std::convertible_to<u64>;
    } || requires(T a) {
        { std::hash<T>{}(a) } -> std::convertible_to<u64>;
    };

    class Hash
    {
    public:
        Hash() = default;

        template <Hashable T>
        constexpr explicit Hash(T const& value) : m_hash(value.hash())
        {
        }

        constexpr bool operator==(Hash const& other) const
        {
            return m_hash == other.m_hash;
        }

        constexpr bool operator!=(Hash const& other) const
        {
            return m_hash != other.m_hash;
        }

        constexpr u64 getValue() const
        {
            return m_hash;
        }

    private:
        u64 m_hash = 0;
    };

    static constexpr u64 hashCombine(u64 seed, u64 x)
    {
        return seed ^ (x + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    }

} // namespace worse::math