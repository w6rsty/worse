#pragma once

#include <cstdint>
#include <concepts>

namespace worse::math
{

    template <typename T>
    concept Hashable = requires(T a) {
        { a.hash() } -> std::convertible_to<std::uint64_t>;
    } || requires(T a) {
        { hash(a) } -> std::convertible_to<std::uint64_t>;
    } || requires(T a) {
        { std::hash<T>{}(a) } -> std::convertible_to<std::uint64_t>;
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

        constexpr std::uint64_t getValue() const
        {
            return m_hash;
        }

    private:
        std::uint64_t m_hash = 0;
    };

    static constexpr std::uint64_t hashCombine(std::uint64_t seed,
                                               std::uint64_t x)
    {
        return seed ^ (x + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    }

} // namespace worse::math