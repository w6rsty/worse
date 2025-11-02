#pragma once
#include "base_type.hpp"
#include "common_macro.hpp"
#include "math/math_constants.hpp"

#include <cmath>
#include <limits>
#include <cstring>
#include <utility>
#include <algorithm>
#include <type_traits>

namespace Worse
{
    namespace comparison
    {

        // T == U
        template <typename T, typename U, typename = void>
        struct HasEqualTo : std::false_type
        {
        };
        template <typename T, typename U>
        struct HasEqualTo<T, U, std::void_t<decltype(std::declval<T>() == std::declval<U>())>> : std::true_type
        {
        };
        template <typename T, typename U>
        inline constexpr Bool HasEqualToV = HasEqualTo<T, U>::value;

        // T != U
        template <typename T, typename U, typename = void>
        struct HasNotEqualTo : std::false_type
        {
        };
        template <typename T, typename U>
        struct HasNotEqualTo<T, U, std::void_t<decltype(std::declval<T>() != std::declval<U>())>> : std::true_type
        {
        };
        template <typename T, typename U>
        inline constexpr Bool HasNotEqualToV = HasNotEqualTo<T, U>::value;

        // T < U
        template <typename T, typename U, typename = void>
        struct HasLess : std::false_type
        {
        };
        template <typename T, typename U>
        struct HasLess<T, U, std::void_t<decltype(std::declval<T>() < std::declval<U>())>> : std::true_type
        {
        };
        template <typename T, typename U>
        inline constexpr Bool HasLessV = HasLess<T, U>::value;

        // T > U
        template <typename T, typename U, typename = void>
        struct HasGreater : std::false_type
        {
        };
        template <typename T, typename U>
        struct HasGreater<T, U, std::void_t<decltype(std::declval<T>() > std::declval<U>())>> : std::true_type
        {
        };
        template <typename T, typename U>
        inline constexpr Bool HasGreaterV = HasGreater<T, U>::value;

        // T <= U
        template <typename T, typename U, typename = void>
        struct HasLessEuqal : std::false_type
        {
        };
        template <typename T, typename U>
        struct HasLessEuqal<T, U, std::void_t<decltype(std::declval<T>() <= std::declval<U>())>> : std::true_type
        {
        };
        template <typename T, typename U>
        inline constexpr Bool HasLessEqualV = HasLessEuqal<T, U>::value;

        // T >= U
        template <typename T, typename U, typename = void>
        struct HasGreaterEqual : std::false_type
        {
        };
        template <typename T, typename U>
        struct HasGreaterEqual<T, U, std::void_t<decltype(std::declval<T>() >= std::declval<U>())>> : std::true_type
        {
        };
        template <typename T, typename U>
        inline constexpr Bool HasGreaterEqualV = HasGreaterEqual<T, U>::value;

        template <typename T>
        struct IsEqualityComparable : std::conjunction<
                                          HasEqualTo<const T&, const T&>,
                                          HasNotEqualTo<const T&, const T&>>
        {
        };

        template <typename T>
        inline constexpr Bool IsEqualityComparableV = IsEqualityComparable<T>::value;

        template <typename T>
        struct IsTotallyOrdered : std::conjunction<
                                      IsEqualityComparable<T>,
                                      HasLess<const T&, const T&>,
                                      HasGreater<const T&, const T&>,
                                      HasLessEuqal<const T&, const T&>,
                                      HasGreaterEqual<const T&, const T&>>
        {
        };

        template <typename T>
        inline constexpr Bool IsTotallyOrderedV = IsTotallyOrdered<T>::value;
    } // namespace comparison

    /*
     * Comparison policies for float pointer
     */

    struct FloatCmpRawBitwise
    {
    };

    struct FloatCmpAbsoluteEps
    {
        Float const epsilon = kFloatEpsilon;
    };

    struct FloatCmpRelativeEps
    {
        Float const epsilon = kFloatEpsilon;
    };

    Bool Eq(Float lhs, Float rhs, FloatCmpRawBitwise);
    Bool Eq(Float lhs, Float rhs, FloatCmpAbsoluteEps policy);
    Bool Eq(Float lhs, Float rhs, FloatCmpRelativeEps policy);

    template <typename T>
    WORSE_FORCE_INLINE Bool Eq(T const& lhs, T const& rhs)
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            return Eq(lhs, rhs, FloatCmpAbsoluteEps{});
        }
        else
        {
            static_assert(comparison::IsEqualityComparableV<T>, "Type must support operator== and operator!=");
            // fall back to common operator==
            return lhs == rhs;
        }
    }

    template <typename T>
    WORSE_FORCE_INLINE Bool Neq(T const& lhs, T const& rhs)
    {
        return !Eq(lhs, rhs);
    }

    template <typename T, typename Policy>
    WORSE_FORCE_INLINE Bool Neq(T const& lhs, T const& rhs, Policy policy)
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            return !Eq(lhs, rhs, policy);
        }
        else
        {
            // fall back to common operator!=
            return Neq(lhs, rhs);
        }
    }

    WORSE_FORCE_INLINE Bool Eq(Float lhs, Float rhs, FloatCmpRawBitwise)
    {
        return std::memcmp(&lhs, &rhs, sizeof(Float)) == 0;
    }

    WORSE_FORCE_INLINE Bool Eq(Float lhs, Float rhs, FloatCmpAbsoluteEps policy)
    {
        return std::abs(lhs - rhs) <= policy.epsilon;
    }

    WORSE_FORCE_INLINE Bool Eq(Float lhs, Float rhs, FloatCmpRelativeEps policy)
    {
        if (Eq(lhs, rhs, FloatCmpAbsoluteEps{}))
        {
            return kTrue;
        }
        return std::abs(lhs - rhs) <= policy.epsilon * std::max(std::abs(lhs), std::abs(rhs));
    }

    template <typename T>
    WORSE_FORCE_INLINE Bool Less(T const& lhs, T const& rhs)
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            return (rhs - lhs) > kFloatEpsilon;
        }
        else
        {
            static_assert(comparison::HasLessV<T, T>, "Type must be totally ordered");
            return lhs < rhs;
        }
    }

    template <typename T>
    WORSE_FORCE_INLINE Bool Greater(T const& lhs, T const& rhs)
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            return (lhs - rhs) > kFloatEpsilon;
        }
        else
        {
            static_assert(comparison::HasGreaterV<T, T>, "Type must be totally ordered");
            return lhs > rhs;
        }
    }

    template <typename T>
    WORSE_FORCE_INLINE Bool LessEq(T const& lhs, T const& rhs)
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            return Less(lhs, rhs) || Eq(lhs, rhs);
        }
        else
        {
            static_assert(comparison::HasLessEqualV<T, T>, "Type must be totally ordered");
            return lhs <= rhs;
        }
    }

    template <typename T>
    WORSE_FORCE_INLINE Bool GreaterEq(T const& lhs, T const& rhs)
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            return Greater(lhs, rhs) || Eq(lhs, rhs);
        }
        else
        {
            static_assert(comparison::HasGreaterEqualV<T, T>, "Type must be totally ordered");
            return lhs >= rhs;
        }
    }

    template <typename T, typename... Ts,
              typename = std::enable_if_t<(comparison::IsTotallyOrderedV<std::decay_t<T>> && ... && comparison::IsTotallyOrderedV<std::decay_t<Ts>>)>>
    WORSE_FORCE_INLINE constexpr std::decay_t<T> Max(T&& first, Ts&&... rest)
    {
        using R  = std::decay_t<T>;
        R result = std::forward<T>(first);
        ((result = result > std::forward<Ts>(rest) ? result : std::forward<Ts>(rest)), ...);
        return result;
    }

    template <typename T, typename... Ts,
              typename = std::enable_if_t<(comparison::IsTotallyOrderedV<std::decay_t<T>> && ... && comparison::IsTotallyOrderedV<std::decay_t<Ts>>)>>
    WORSE_FORCE_INLINE constexpr std::decay_t<T> Min(T&& first, Ts&&... rest)
    {
        using R  = std::decay_t<T>;
        R result = std::forward<T>(first);
        ((result = result < std::forward<Ts>(rest) ? result : std::forward<Ts>(rest)), ...);
        return result;
    }

} // namespace Worse