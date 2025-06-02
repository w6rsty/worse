#pragma once
#include <type_traits>

namespace worse
{
    struct NonCopyable
    {
        NonCopyable()                              = default;
        NonCopyable(NonCopyable const&)            = delete;
        NonCopyable& operator=(NonCopyable const&) = delete;
        NonCopyable(NonCopyable&&)                 = default;
        NonCopyable& operator=(NonCopyable&&)      = default;
    };

    struct NonMovable
    {
        NonMovable()                             = default;
        NonMovable(NonMovable const&)            = delete;
        NonMovable& operator=(NonMovable const&) = delete;
        NonMovable(NonMovable&&)                 = delete;
        NonMovable& operator=(NonMovable&&)      = delete;
    };

    /// Helper class to define bit flags.
    /// Usage:
    ///     struct State
    ///     {
    ///         using ValueType = int;
    ///     };
    ///     using StateFlags = Flags<State>;
    /// Or:
    ///     WS_DEFINE_FLAGS(State, int);
    ///
    ///  Then you can define flag bits like this:
    ///     struct StateFlagBits
    ///     {
    ///         static constexpr StateFlags A{0b0000'0001};
    ///         static constexpr StateFlags B{0b0000'0010};
    ///         static constexpr StateFlags C{0b0000'0100};
    ///     };
    template <typename Category>
        requires(std::is_integral_v<typename Category::ValueType>)
    struct Flags
    {
        using ValueType = typename Category::ValueType;
        ValueType value;

        constexpr Flags& operator|=(Flags const& rhs)
        {
            value |= rhs.value;
            return *this;
        }

        constexpr Flags& operator&=(Flags const& rhs)
        {
            value &= rhs.value;
            return *this;
        }

        constexpr Flags& operator^=(Flags const& rhs)
        {
            value ^= rhs.value;
            return *this;
        }

        [[nodiscard]] constexpr Flags operator~() const
        {
            return Flags{static_cast<ValueType>(~value)};
        }

        [[nodiscard]] constexpr Flags operator|(Flags const& rhs) const
        {
            return Flags{static_cast<ValueType>(value | rhs.value)};
        }

        [[nodiscard]] constexpr Flags operator&(Flags const& rhs) const
        {
            return Flags{static_cast<ValueType>(value & rhs.value)};
        }

        [[nodiscard]] constexpr Flags operator^(Flags const& rhs) const
        {
            return Flags{static_cast<ValueType>(value ^ rhs.value)};
        }

        [[nodiscard]] constexpr auto operator<=>(Flags const& rhs) const = default;

        [[nodiscard]] explicit operator bool() const
        {
            return value != 0;
        }
    };

#define WS_DEFINE_FLAGS(name, base)                                                                \
    struct name                                                                                    \
    {                                                                                              \
        static_assert(std::is_integral_v<base>, "Flags base type must be integral");               \
        using ValueType = base;                                                                    \
    };                                                                                             \
    using name##Flags = Flags<name>;

    /// Helper function to iterate over each flag bit.
    /// Usage:
    ///     StateFlags flags = StateFlagBits::A | StateFlagBits::B;
    ///     flagsForEach(flags, [](StateFlags flag) {
    ///         // Do something with each flag
    ///     });
    template <typename Category, typename Func>
    void flagsForEach(Flags<Category> flags, Func&& func)
    {
        using ValueType = typename Flags<Category>::ValueType;
        ValueType bits  = flags.value;

        while (bits)
        {
            ValueType single = bits & -bits;
            func(Flags<Category>{single});
            bits &= bits - 1;
        }
    }

} // namespace worse