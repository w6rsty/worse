#pragma once
#include "base_type.hpp"

#include <iterator>
#include <type_traits>

namespace Worse
{

    namespace Common::Detail
    {

        template <typename BaseType, typename FlagBitType>
        class FlagBitIterator
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type        = FlagBitType;
            using difference_type   = PtrDiff;
            using pointer           = value_type const*;
            using reference         = value_type const&;

            constexpr explicit FlagBitIterator(BaseType bits)
                : m_bits{bits}, m_current{ExtractLowestBit(bits)}
            {
            }

            constexpr FlagBitIterator()
                : m_bits{BaseType{}}, m_current{FlagBitType{}}
            {
            }

            constexpr reference operator*() const
            {
                return m_current;
            }
            constexpr pointer operator->() const
            {
                return &m_current;
            }

            constexpr FlagBitIterator& operator++()
            {
                if (m_bits != 0)
                {
                    // clear current lowest bit
                    m_bits &= (m_bits - 1);
                    // extract next lowest bit
                    m_current = FlagBitType{ExtractLowestBit(m_bits)};
                }
                return *this;
            }

            constexpr FlagBitIterator operator++(Int)
            {
                FlagBitIterator tmp = *this;
                ++(*this);
                return tmp;
            }

            constexpr Bool operator==(FlagBitIterator const& rhs) const
            {
                return m_bits == rhs.m_bits;
            }

            constexpr Bool operator!=(FlagBitIterator const& rhs) const
            {
                return !(*this == rhs);
            }

        private:
            static constexpr BaseType ExtractLowestBit(BaseType value)
            {
                return value & static_cast<BaseType>(-value);
            }

        private:
            BaseType m_bits;
            FlagBitType m_current;
        };

        template <typename BaseType>
        class FlagBit
        {
        public:
            using Iterator = Common::Detail::FlagBitIterator<BaseType, FlagBit>;

            // clang-format off
            constexpr FlagBit() : m_value{BaseType{}} {}
            explicit constexpr FlagBit(BaseType value) : m_value{value} {}

            constexpr FlagBit& operator|=(FlagBit const& rhs){ m_value |= rhs.m_value; return *this; }
            constexpr FlagBit& operator&=(FlagBit const& rhs) { m_value &= rhs.m_value; return *this; }
            constexpr FlagBit& operator^=(FlagBit const& rhs) { m_value ^= rhs.m_value; return *this; }

            [[nodiscard]] constexpr FlagBit operator~() const                   { return FlagBit{static_cast<BaseType>(~m_value)}; }
            [[nodiscard]] constexpr FlagBit operator|(FlagBit const& rhs) const { return FlagBit{static_cast<BaseType>(m_value | rhs.m_value)}; }
            [[nodiscard]] constexpr FlagBit operator&(FlagBit const& rhs) const { return FlagBit{static_cast<BaseType>(m_value & rhs.m_value)}; }
            [[nodiscard]] constexpr FlagBit operator^(FlagBit const& rhs) const { return FlagBit{static_cast<BaseType>(m_value ^ rhs.m_value)}; }
    
            [[nodiscard]] explicit operator Bool() const { return m_value != 0; }
            [[nodiscard]] operator BaseType() const      { return m_value; }
        
            [[nodiscard]] constexpr Bool operator==(FlagBit const& rhs) const { return m_value == rhs.m_value; }
            [[nodiscard]] constexpr Bool operator!=(FlagBit const& rhs) const { return m_value != rhs.m_value; }
            [[nodiscard]] constexpr Bool operator<(FlagBit const& rhs) const  { return m_value < rhs.m_value; }
            [[nodiscard]] constexpr Bool operator<=(FlagBit const& rhs) const { return m_value <= rhs.m_value; }
            [[nodiscard]] constexpr Bool operator>(FlagBit const& rhs) const  { return m_value > rhs.m_value; }
            [[nodiscard]] constexpr Bool operator>=(FlagBit const& rhs) const { return m_value >= rhs.m_value; }

            // Get underlying value
            [[nodiscard]] constexpr BaseType Value() const { return m_value; }

            // Const range-for support
            [[nodiscard]] constexpr Iterator begin() const { return Iterator{m_value}; }
            [[nodiscard]] constexpr Iterator end() const   { return Iterator{}; }
            // clang-format on

        public:
            BaseType m_value;
        };

    } // namespace Common::Detail

    /// Usage:
    ///
    /// class State final : public BitFlag<Int>
    /// {
    /// public:
    ///     struct FlagBits
    ///     {
    ///         static constexpr State::Bit A{0b0000'0001};
    ///         static constexpr State::Bit B{0b0000'0010};
    ///         static constexpr State::Bit C{0b0000'0100};
    ///     };
    /// };
    ///
    /// State::Flags state = State::FlagBits::A | State::FlagBits::C;
    ///
    /// Or use helper macro:
    ///
    /// WORSE_BEGIN_DECLARE_BIT_FLAG(State, Int)
    /// WORSE_DECLARE_FLAG_BIT(A, 0b0000'0001)
    /// WORSE_DECLARE_FLAG_BIT(B, 0b0000'0010)
    /// WORSE_DECLARE_FLAG_BIT(C, 0b0000'0100)
    /// WORSE_END_DECLARE_BIT_FLAG(State)
    ///
    /// Const range-for:
    ///
    /// for (State::Bit flag : state)
    /// {
    ///     std::printf("Flag value: 0x%02X\n", static_cast<int>(flag));
    /// }
    template <typename BaseType>
    class BitFlag
    {
        static_assert(std::is_integral_v<BaseType>, "BaseType must be an integral type");

    protected:
        // Type to declare flag bit
        using Bit = Common::Detail::FlagBit<BaseType>;

    public:
        using Flags = Common::Detail::FlagBit<BaseType>;

        static constexpr Flags FromValue(BaseType value)
        {
            return Flags{value};
        }
    };

#define WORSE_BEGIN_DECLARE_BIT_FLAG(typeName, baseType)     \
    class typeName final : public ::Worse::BitFlag<baseType> \
    {                                                        \
    public:                                                  \
        struct FlagBits                                      \
        {

#define WORSE_END_DECLARE_BIT_FLAG(typeName) \
    }                                        \
    ;                                        \
    }                                        \
    ;

#define WORSE_DECLARE_FLAG_BIT(bitName, bitValue) \
    static constexpr Bit bitName{bitValue};

} // namespace Worse