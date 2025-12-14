#pragma once

#include "macro/common_macro.hpp"

#include <type_traits>

namespace Worse
{

    template <typename BaseType, typename = std::enable_if_t<std::is_unsigned_v<BaseType>>>
    class TFlagBit
    {
    public:
        constexpr TFlagBit()
            : m_value(BaseType())
        {
        }

        constexpr TFlagBit(BaseType value)
            : m_value(value)
        {
        }

        WORSE_FORCE_INLINE constexpr TFlagBit& operator|=(TFlagBit const& rhs)
        {
            m_value |= rhs.m_value;
            return *this;
        }
        WORSE_FORCE_INLINE constexpr TFlagBit& operator&=(TFlagBit const& rhs)
        {
            m_value &= rhs.m_value;
            return *this;
        }
        WORSE_FORCE_INLINE constexpr TFlagBit& operator^=(TFlagBit const& rhs)
        {
            m_value ^= rhs.m_value;
            return *this;
        }

        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr TFlagBit operator~() const { return TFlagBit{static_cast<BaseType>(~m_value)}; }
        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr TFlagBit operator|(TFlagBit const& rhs) const { return TFlagBit{static_cast<BaseType>(m_value | rhs.m_value)}; }
        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr TFlagBit operator&(TFlagBit const& rhs) const { return TFlagBit{static_cast<BaseType>(m_value & rhs.m_value)}; }
        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr TFlagBit operator^(TFlagBit const& rhs) const { return TFlagBit{static_cast<BaseType>(m_value ^ rhs.m_value)}; }

        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr bool operator==(TFlagBit const& rhs) const { return m_value == rhs.m_value; }
        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr bool operator!=(TFlagBit const& rhs) const { return m_value != rhs.m_value; }
        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr bool operator<(TFlagBit const& rhs) const { return m_value < rhs.m_value; }
        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr bool operator<=(TFlagBit const& rhs) const { return m_value <= rhs.m_value; }
        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr bool operator>(TFlagBit const& rhs) const { return m_value > rhs.m_value; }
        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr bool operator>=(TFlagBit const& rhs) const { return m_value >= rhs.m_value; }

        WORSE_NODISCARD operator bool() const { return m_value != 0; }
        WORSE_NODISCARD operator BaseType() const { return m_value; }

        WORSE_NODISCARD constexpr BaseType GetValue() const { return m_value; }

    private:
        BaseType m_value;

    public:
        /**
         * @brief Consuming bit iterator.
         */
        class BitIterator
        {
        public:
            constexpr explicit BitIterator(BaseType bits)
                : m_bits(bits), m_current(0), m_isValid(false)
            {
                Advance();
            }

            WORSE_NODISCARD constexpr BaseType operator*() const
            {
                return m_current;
            }

            constexpr BitIterator& operator++()
            {
                Advance();
                return *this;
            }

            WORSE_NODISCARD constexpr bool operator!=(BitIterator const& rhs) const
            {
                return m_isValid;
            }

        private:
            WORSE_FORCE_INLINE constexpr void Advance()
            {
                if (m_bits == 0)
                {
                    m_isValid = false;
                    return;
                }

                m_isValid = true;

                m_current = LeastSignificantBit(m_bits);

                m_bits &= ~m_current;
            }

            WORSE_NODISCARD WORSE_FORCE_INLINE static constexpr BaseType LeastSignificantBit(BaseType value)
            {
                return value & -value;
            }

        private:
            BaseType m_bits;
            BaseType m_current;
            bool m_isValid;
        };

        using RangedForIteratorType = BitIterator;

        RangedForIteratorType constexpr begin() { return RangedForIteratorType(m_value); }
        RangedForIteratorType constexpr end() { return RangedForIteratorType(0); }
    };

    /**
     * @brief Bit flag type.
     * 
     * @tparam BaseType Underlying value type
     */
    template <typename BaseType>
    class TBitFlag
    {
    public:
        using BitType  = typename TFlagBit<BaseType>;
        using FlagType = typename TFlagBit<BaseType>;

        static constexpr TBitFlag FromValue(BaseType value)
        {
            return Flags(value);
        }
    };

} // namespace Worse