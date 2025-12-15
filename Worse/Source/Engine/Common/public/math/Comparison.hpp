#pragma once

#include "Macro/Common.hpp"
#include "Math/Math.hpp"

#include <cstring>
#include <utility>
#include <algorithm>
#include <type_traits>

namespace worse
{

    namespace Comparison
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
        constexpr bool HasEqualTo_V = HasEqualTo<T, U>::value;

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
        constexpr bool HasNotEqualTo_V = HasNotEqualTo<T, U>::value;

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
        constexpr bool HasLess_V = HasLess<T, U>::value;

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
        constexpr bool HasGreater_V = HasGreater<T, U>::value;

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
        constexpr bool HasLessEqual_V = HasLessEuqal<T, U>::value;

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
        constexpr bool HasGreaterEqual_V = HasGreaterEqual<T, U>::value;

        template <typename T>
        struct IsEqualityComparable : std::conjunction<
                                          HasEqualTo<const T&, const T&>,
                                          HasNotEqualTo<const T&, const T&>>
        {
        };

        template <typename T>
        constexpr bool IsEqualityComparable_V = IsEqualityComparable<T>::value;

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
        constexpr bool IsTotallyOrdered_V = IsTotallyOrdered<T>::value;
    } // namespace Comparison

    /*
     * Comparison policies for float pointer
     */

    struct FloatCmpRawBitwise
    {
    };

    struct FloatCmpAbsoluteEps
    {
        F32 const epsilon = FMath::kFloatEpsilon;
    };

    struct FloatCmpRelativeEps
    {
        F32 const epsilon = FMath::kFloatEpsilon;
    };

    bool Eq(F32 lhs, F32 rhs, FloatCmpRawBitwise);
    bool Eq(F32 lhs, F32 rhs, FloatCmpAbsoluteEps policy);
    bool Eq(F32 lhs, F32 rhs, FloatCmpRelativeEps policy);

    template <typename T>
    WORSE_FORCE_INLINE bool Eq(T const& lhs, T const& rhs)
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            return Eq(lhs, rhs, FloatCmpAbsoluteEps{});
        }
        else
        {
            static_assert(Comparison::IsEqualityComparable_V<T>, "Type must support operator== and operator!=");
            // fall back to common operator==
            return lhs == rhs;
        }
    }

    template <typename T>
    WORSE_FORCE_INLINE bool Neq(T const& lhs, T const& rhs)
    {
        return !Eq(lhs, rhs);
    }

    template <typename T, typename Policy>
    WORSE_FORCE_INLINE bool Neq(T const& lhs, T const& rhs, Policy policy)
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

    WORSE_FORCE_INLINE bool Eq(F32 lhs, F32 rhs, FloatCmpRawBitwise)
    {
        return std::memcmp(&lhs, &rhs, sizeof(F32)) == 0;
    }

    WORSE_FORCE_INLINE bool Eq(F32 lhs, F32 rhs, FloatCmpAbsoluteEps policy)
    {
        return std::abs(lhs - rhs) <= policy.epsilon;
    }

    WORSE_FORCE_INLINE bool Eq(F32 lhs, F32 rhs, FloatCmpRelativeEps policy)
    {
        if (Eq(lhs, rhs, FloatCmpAbsoluteEps{}))
        {
            return true;
        }
        return std::abs(lhs - rhs) <= policy.epsilon * std::max(std::abs(lhs), std::abs(rhs));
    }

    template <typename T>
    WORSE_FORCE_INLINE bool Less(T const& lhs, T const& rhs)
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            return (rhs - lhs) > FMath::kFloatEpsilon;
        }
        else
        {
            static_assert(Comparison::HasLess_V<T, T>, "Type must be totally ordered");
            return lhs < rhs;
        }
    }

    template <typename T>
    WORSE_FORCE_INLINE bool Greater(T const& lhs, T const& rhs)
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            return (lhs - rhs) > FMath::kFloatEpsilon;
        }
        else
        {
            static_assert(Comparison::HasGreater_V<T, T>, "Type must be totally ordered");
            return lhs > rhs;
        }
    }

    template <typename T>
    WORSE_FORCE_INLINE bool LessEq(T const& lhs, T const& rhs)
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            return Less(lhs, rhs) || Eq(lhs, rhs);
        }
        else
        {
            static_assert(Comparison::HasLessEqual_V<T, T>, "Type must be totally ordered");
            return lhs <= rhs;
        }
    }

    template <typename T>
    WORSE_FORCE_INLINE bool GreaterEq(T const& lhs, T const& rhs)
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            return Greater(lhs, rhs) || Eq(lhs, rhs);
        }
        else
        {
            static_assert(Comparison::HasGreaterEqual_V<T, T>, "Type must be totally ordered");
            return lhs >= rhs;
        }
    }

    template <typename T, typename... Ts,
              typename = std::enable_if_t<(Comparison::IsTotallyOrdered_V<std::decay_t<T>> && ... && Comparison::IsTotallyOrdered_V<std::decay_t<Ts>>)>>
    WORSE_FORCE_INLINE constexpr std::decay_t<T> Max(T&& first, Ts&&... rest)
    {
        using R  = std::decay_t<T>;
        R result = std::forward<T>(first);
        ((result = result > std::forward<Ts>(rest) ? result : std::forward<Ts>(rest)), ...);
        return result;
    }

    template <typename T, typename... Ts,
              typename = std::enable_if_t<(Comparison::IsTotallyOrdered_V<std::decay_t<T>> && ... && Comparison::IsTotallyOrdered_V<std::decay_t<Ts>>)>>
    WORSE_FORCE_INLINE constexpr std::decay_t<T> Min(T&& first, Ts&&... rest)
    {
        using R  = std::decay_t<T>;
        R result = std::forward<T>(first);
        ((result = result < std::forward<Ts>(rest) ? result : std::forward<Ts>(rest)), ...);
        return result;
    }

} // namespace worse