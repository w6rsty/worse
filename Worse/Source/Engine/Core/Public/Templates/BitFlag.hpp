#pragma once

#include "Macro/Common.hpp"

#include <type_traits>

namespace worse
{

    template <typename BaseType, typename = std::enable_if_t<std::is_unsigned_v<BaseType>>>
    class TFlagBit
    {
    public:
        constexpr TFlagBit()
            : m_Value(BaseType())
        {
        }

        constexpr TFlagBit(BaseType value)
            : m_Value(value)
        {
        }

        WORSE_FORCE_INLINE constexpr TFlagBit& operator|=(TFlagBit const& rhs)
        {
            m_Value |= rhs.m_Value;
            return *this;
        }
        WORSE_FORCE_INLINE constexpr TFlagBit& operator&=(TFlagBit const& rhs)
        {
            m_Value &= rhs.m_Value;
            return *this;
        }
        WORSE_FORCE_INLINE constexpr TFlagBit& operator^=(TFlagBit const& rhs)
        {
            m_Value ^= rhs.m_Value;
            return *this;
        }

        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr TFlagBit operator~() const { return TFlagBit{static_cast<BaseType>(~m_Value)}; }
        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr TFlagBit operator|(TFlagBit const& rhs) const { return TFlagBit{static_cast<BaseType>(m_Value | rhs.m_Value)}; }
        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr TFlagBit operator&(TFlagBit const& rhs) const { return TFlagBit{static_cast<BaseType>(m_Value & rhs.m_Value)}; }
        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr TFlagBit operator^(TFlagBit const& rhs) const { return TFlagBit{static_cast<BaseType>(m_Value ^ rhs.m_Value)}; }

        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr bool operator==(TFlagBit const& rhs) const { return m_Value == rhs.m_Value; }
        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr bool operator!=(TFlagBit const& rhs) const { return m_Value != rhs.m_Value; }
        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr bool operator<(TFlagBit const& rhs) const { return m_Value < rhs.m_Value; }
        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr bool operator<=(TFlagBit const& rhs) const { return m_Value <= rhs.m_Value; }
        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr bool operator>(TFlagBit const& rhs) const { return m_Value > rhs.m_Value; }
        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr bool operator>=(TFlagBit const& rhs) const { return m_Value >= rhs.m_Value; }

        WORSE_NODISCARD operator bool() const { return m_Value != 0; }
        WORSE_NODISCARD operator BaseType() const { return m_Value; }

        WORSE_NODISCARD constexpr BaseType GetValue() const { return m_Value; }

    private:
        BaseType m_Value;

    public:
        /**
         * @brief Consuming bit iterator.
         */
        class BitIterator
        {
        public:
            constexpr explicit BitIterator(BaseType bits)
                : m_Bits(bits), m_Current(0), m_IsValid(false)
            {
                Advance();
            }

            WORSE_NODISCARD constexpr BaseType operator*() const
            {
                return m_Current;
            }

            constexpr BitIterator& operator++()
            {
                Advance();
                return *this;
            }

            WORSE_NODISCARD constexpr bool operator!=(BitIterator const& rhs) const
            {
                return m_IsValid;
            }

        private:
            WORSE_FORCE_INLINE constexpr void Advance()
            {
                if (m_Bits == 0)
                {
                    m_IsValid = false;
                    return;
                }

                m_IsValid = true;

                m_Current = LeastSignificantBit(m_Bits);

                m_Bits &= ~m_Current;
            }

            WORSE_NODISCARD WORSE_FORCE_INLINE static constexpr BaseType LeastSignificantBit(BaseType value)
            {
                return value & -value;
            }

        private:
            BaseType m_Bits;
            BaseType m_Current;
            bool m_IsValid;
        };

        using RangedForIteratorType = BitIterator;

        RangedForIteratorType constexpr begin() { return RangedForIteratorType(m_Value); }
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

} // namespace worse