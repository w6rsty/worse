#pragma once
#include "base_type.hpp"

#include <concepts>

namespace Worse::math
{

    template <typename T>
    concept Hashable = requires(T a) {
        { a.hash() } -> std::convertible_to<ULong>;
    } || requires(T a) {
        { hash(a) } -> std::convertible_to<ULong>;
    } || requires(T a) {
        { std::hash<T>{}(a) } -> std::convertible_to<ULong>;
    };

    class Hash
    {
    public:
        Hash() = default;

        template <Hashable T>
        constexpr explicit Hash(T const& value)
            : m_hash(value.hash())
        {
        }

        constexpr Bool operator==(Hash const& other) const
        {
            return m_hash == other.m_hash;
        }

        constexpr Bool operator!=(Hash const& other) const
        {
            return m_hash != other.m_hash;
        }

        constexpr ULong getValue() const
        {
            return m_hash;
        }

    private:
        ULong m_hash = 0;
    };

    static constexpr ULong hashCombine(ULong seed, ULong x)
    {
        return seed ^ (x + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    }

} // namespace Worse::math