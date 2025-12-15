#pragma once

#include "BaseTypes.hpp"

#include <concepts>

namespace worse::math
{

    template <typename T>
    concept Hashable = requires(T a) {
        { a.hash() } -> std::convertible_to<U64>;
    } || requires(T a) {
        { hash(a) } -> std::convertible_to<U64>;
    } || requires(T a) {
        { std::hash<T>{}(a) } -> std::convertible_to<U64>;
    };

    class Hash
    {
    public:
        Hash() = default;

        template <Hashable T>
        constexpr explicit Hash(T const& value)
            : m_HashValue(value.hash())
        {
        }

        constexpr bool operator==(Hash const& other) const
        {
            return m_HashValue == other.m_HashValue;
        }

        constexpr bool operator!=(Hash const& other) const
        {
            return m_HashValue != other.m_HashValue;
        }

        constexpr U64 GetValue() const
        {
            return m_HashValue;
        }

    private:
        U64 m_HashValue = 0;
    };

    static constexpr U64 HashCombine(U64 seed, U64 x)
    {
        return seed ^ (x + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    }

} // namespace worse::math