#pragma once
#include "base_type.hpp"
#include "macro/common_macro.hpp"
#include "container/container_define.hpp"
#include "container/static_array.hpp"

#include <type_traits>

namespace Worse
{
    template <
        typename InEnumType,
        typename InElementType,
        UInt NumEnums = s_cast<UInt>(InEnumType::Max),
        typename      = std::enable_if_t<std::is_enum_v<InEnumType>>>
    class TEnumArray
    {
    public:
        using EnumType    = InElementType;
        using ElementType = InElementType;

    private:
        using StorageType = TStaticArray<InElementType, NumEnums>;

    public:
        constexpr TEnumArray() = default;

        // Constructs each element with args
        template <typename... ArgTypes>
        constexpr explicit TEnumArray(EInPlace, ArgTypes&&... args)
            : m_data(InPlace, std::forward<ArgTypes>(args)...)
        {
        }

        constexpr TEnumArray(TEnumArray const&)            = default;
        constexpr TEnumArray(TEnumArray&&)                 = default;
        constexpr TEnumArray& operator=(TEnumArray const&) = default;
        constexpr TEnumArray& operator=(TEnumArray&&)      = default;

        // Accessors
        WORSE_NODISCARD constexpr InElementType& operator[](InEnumType index)
        {
            return m_data[s_cast<UInt>(index)];
        }

        WORSE_NODISCARD constexpr InElementType const& operator[](InEnumType index) const
        {
            return m_data[s_cast<UInt>(index)];
        }

        // Comparisons
        WORSE_NODISCARD constexpr friend Bool operator==(TEnumArray const& lhs, TEnumArray const& rhs)
        {
            for (UInt elementIndex = 0; elementIndex < NumEnums; ++elementIndex)
            {
                if (!(lhs.m_data[elementIndex] == rhs.m_data[elementIndex]))
                {
                    return kFalse;
                }
            }
            return kTrue;
        }

        WORSE_NODISCARD constexpr Bool operator!=(TEnumArray const& other) const
        {
            for (UInt elementIndex = 0; elementIndex < NumEnums; ++elementIndex)
            {
                if (!((this->m_data)[elementIndex] == other.m_data[elementIndex]))
                {
                    return kTrue;
                }
            }
            return kFalse;
        }

        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr Int Num() const
        {
            return s_cast<Int>(NumEnums);
        }

    private:
        StorageType m_data;

    public:
        using RangedForIteratorType             = StorageType::RangedForIteratorType;
        using RangedForConstIteratorType        = StorageType::RangedForConstIteratorType;
        using RangedForReverseIteratorType      = StorageType::RangedForReverseIteratorType;
        using RangedForConstReverseIteratorType = StorageType::RangedForConstReverseIteratorType;

        // clang-format off
        RangedForIteratorType             constexpr begin()        { return m_data.begin(); }
        RangedForConstIteratorType        constexpr begin()  const { return m_data.begin(); }
        RangedForIteratorType             constexpr end()          { return m_data.end(); }
        RangedForConstIteratorType        constexpr end()    const { return m_data.end(); }
        RangedForReverseIteratorType      constexpr rbegin()       { return m_data.rbegin(); }
        RangedForConstReverseIteratorType constexpr rbegin() const { return m_data.rbegin(); }
        RangedForReverseIteratorType      constexpr rend()         { return m_data.rend(); }
        RangedForConstReverseIteratorType constexpr rend()   const { return m_data.rend(); }
        // clang-format on
    };

} // namespace Worse