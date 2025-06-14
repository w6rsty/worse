#pragma once
#include <array>
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

        // clang-format off
        constexpr Flags& operator|=(Flags const& rhs){ value |= rhs.value; return *this; }
        constexpr Flags& operator&=(Flags const& rhs) { value &= rhs.value; return *this; }
        constexpr Flags& operator^=(Flags const& rhs) { value ^= rhs.value; return *this; }

        [[nodiscard]] constexpr Flags operator~() const                 { return Flags{static_cast<ValueType>(~value)}; }
        [[nodiscard]] constexpr Flags operator|(Flags const& rhs) const { return Flags{static_cast<ValueType>(value | rhs.value)}; }
        [[nodiscard]] constexpr Flags operator&(Flags const& rhs) const { return Flags{static_cast<ValueType>(value & rhs.value)}; }
        [[nodiscard]] constexpr Flags operator^(Flags const& rhs) const { return Flags{static_cast<ValueType>(value ^ rhs.value)}; }
        [[nodiscard]] constexpr auto operator<=>(Flags const& rhs) const = default;
        [[nodiscard]] explicit operator bool() const { return value != 0; }
        [[nodiscard]] operator ValueType() const { return value; }
        // clang-format on
    };

#define WS_DEFINE_FLAGS(name, base)                                            \
    struct name                                                                \
    {                                                                          \
        static_assert(std::is_integral_v<base>,                                \
                      "Flags base type must be integral");                     \
        using ValueType = base;                                                \
    };                                                                         \
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

    template <typename E>
    concept EnumArrayConcept = std::is_enum_v<E> && requires { E::Max; };

    template <EnumArrayConcept E, typename T> class EnumArray
    {
    public:
        static constexpr std::size_t N = static_cast<std::size_t>(E::Max);

        using value_type      = T;
        using size_type       = std::size_t;
        using reference       = T&;
        using const_reference = T const&;
        using iterator        = typename std::array<T, N>::iterator;
        using const_iterator  = typename std::array<T, N>::const_iterator;

        // clang-format off
        constexpr reference       operator[](std::size_t index)       { return m_data[index]; }
        constexpr reference       operator[](E index)                 { return m_data[static_cast<std::size_t>(index)]; }
        constexpr const_reference operator[](std::size_t index) const { return m_data[index]; }
        constexpr const_reference operator[](E index) const           { return m_data[static_cast<std::size_t>(index)]; }
        constexpr reference       at(E index)                         { return m_data.at(static_cast<std::size_t>(index)); }
        constexpr const_reference at(E index) const                   { return m_data.at(static_cast<std::size_t>(index)); }
        constexpr iterator        begin() noexcept                    { return m_data.begin(); }
        constexpr const_iterator  begin() const noexcept              { return m_data.begin(); }
        constexpr const_iterator  cbegin() const noexcept             { return m_data.cbegin(); }
        constexpr iterator        end() noexcept                      { return m_data.end(); }
        constexpr const_iterator  end() const noexcept                { return m_data.end(); }
        constexpr const_iterator  cend() const noexcept               { return m_data.cend(); }
        constexpr size_type       size() const noexcept               { return N; }
        constexpr bool            empty() const noexcept              { return N == 0; }
        constexpr reference       front()                             { return m_data.front(); }
        constexpr const_reference front() const                       { return m_data.front(); }
        constexpr reference       back()                              { return m_data.back(); }
        constexpr const_reference back() const                        { return m_data.back(); }
        constexpr T*              data() noexcept                     { return m_data.data(); }
        constexpr T const*        data() const noexcept               { return m_data.data(); }
        constexpr void            fill(const T& value)                { m_data.fill(value); }
        // clang-format on

        class iterator_pair
        {
        public:
            using difference_type   = std::ptrdiff_t;
            using value_type        = std::pair<E, std::reference_wrapper<T>>;
            using pointer           = void;
            using reference         = std::pair<E, std::reference_wrapper<T>>;
            using iterator_category = std::forward_iterator_tag;

            constexpr iterator_pair(typename std::array<T, N>::iterator it,
                                    std::size_t index)
                : m_it(it), m_index(index)
            {
            }

            constexpr reference operator*() const
            {
                return {static_cast<E>(m_index), std::ref(*m_it)};
            }

            constexpr iterator_pair& operator++()
            {
                ++m_it;
                ++m_index;
                return *this;
            }

            constexpr iterator_pair operator++(int)
            {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            constexpr bool operator==(const iterator_pair& other) const
            {
                return m_it == other.m_it;
            }

            constexpr bool operator!=(const iterator_pair& other) const
            {
                return !(*this == other);
            }

        private:
            typename std::array<T, N>::iterator m_it;
            std::size_t m_index;
        };

        class const_iterator_pair
        {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = std::pair<E, std::reference_wrapper<const T>>;
            using pointer    = void;
            using reference  = std::pair<E, std::reference_wrapper<const T>>;
            using iterator_category = std::forward_iterator_tag;

            constexpr const_iterator_pair(
                typename std::array<T, N>::const_iterator it, std::size_t index)
                : m_it(it), m_index(index)
            {
            }

            constexpr reference operator*() const
            {
                return {static_cast<E>(m_index), std::cref(*m_it)};
            }

            constexpr const_iterator_pair& operator++()
            {
                ++m_it;
                ++m_index;
                return *this;
            }

            constexpr const_iterator_pair operator++(int)
            {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            constexpr bool operator==(const const_iterator_pair& other) const
            {
                return m_it == other.m_it;
            }

            constexpr bool operator!=(const const_iterator_pair& other) const
            {
                return !(*this == other);
            }

        private:
            typename std::array<T, N>::const_iterator m_it;
            std::size_t m_index;
        };

        // clang-format off
        constexpr iterator_pair       begin_pairs() noexcept        { return iterator_pair{m_data.begin(), 0}; }
        constexpr const_iterator_pair begin_pairs() const noexcept  { return const_iterator_pair{m_data.begin(), 0}; }
        constexpr const_iterator_pair cbegin_pairs() const noexcept { return const_iterator_pair{m_data.cbegin(), 0}; }
        constexpr iterator_pair       end_pairs() noexcept          { return iterator_pair{m_data.end(), N}; }
        constexpr const_iterator_pair end_pairs() const noexcept    { return const_iterator_pair{m_data.end(), N}; }
        constexpr const_iterator_pair cend_pairs() const noexcept   { return const_iterator_pair{m_data.cend(), N}; }
        // clang-format on

    private:
        std::array<T, N> m_data{};
    };

} // namespace worse