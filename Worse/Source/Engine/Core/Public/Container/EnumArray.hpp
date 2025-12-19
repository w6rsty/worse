#pragma once

#include "Macro/Common.hpp"
#include "Container/ContainerDefines.hpp"
#include "Container/StaticArray.hpp"

#include <type_traits>

namespace worse
{
    template <
        typename InEnumType,
        typename InElementType,
        U32 NumEnums = static_cast<U32>(InEnumType::Count),
        typename     = std::enable_if_t<std::is_enum_v<InEnumType>>>
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
            : m_Data(InPlace, std::forward<ArgTypes>(args)...)
        {
        }

        TEnumArray(TEnumArray const&)            = default;
        TEnumArray(TEnumArray&&)                 = default;
        TEnumArray& operator=(TEnumArray const&) = default;
        TEnumArray& operator=(TEnumArray&&)      = default;

        // Accessors
        WORSE_NODISCARD constexpr InElementType& operator[](InEnumType index)
        {
            return m_Data[static_cast<U32>(index)];
        }

        WORSE_NODISCARD constexpr InElementType const& operator[](InEnumType index) const
        {
            return m_Data[static_cast<U32>(index)];
        }

        // Comparisons
        WORSE_NODISCARD constexpr friend bool operator==(TEnumArray const& lhs, TEnumArray const& rhs)
        {
            for (U32 elementIndex = 0; elementIndex < NumEnums; ++elementIndex)
            {
                if (!(lhs.m_Data[elementIndex] == rhs.m_Data[elementIndex]))
                {
                    return false;
                }
            }
            return true;
        }

        WORSE_NODISCARD constexpr bool operator!=(TEnumArray const& other) const
        {
            for (U32 elementIndex = 0; elementIndex < NumEnums; ++elementIndex)
            {
                if (!((this->m_Data)[elementIndex] == other.m_Data[elementIndex]))
                {
                    return true;
                }
            }
            return true;
        }

        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr I32 Num() const
        {
            return static_cast<I32>(NumEnums);
        }

    private:
        StorageType m_Data;

    public:
        using RangedForIteratorType             = StorageType::RangedForIteratorType;
        using RangedForConstIteratorType        = StorageType::RangedForConstIteratorType;
        using RangedForReverseIteratorType      = StorageType::RangedForReverseIteratorType;
        using RangedForConstReverseIteratorType = StorageType::RangedForConstReverseIteratorType;

        // clang-format off
        RangedForIteratorType             constexpr begin()        { return m_Data.begin(); }
        RangedForConstIteratorType        constexpr begin()  const { return m_Data.begin(); }
        RangedForIteratorType             constexpr end()          { return m_Data.end(); }
        RangedForConstIteratorType        constexpr end()    const { return m_Data.end(); }
        RangedForReverseIteratorType      constexpr rbegin()       { return m_Data.rbegin(); }
        RangedForConstReverseIteratorType constexpr rbegin() const { return m_Data.rbegin(); }
        RangedForReverseIteratorType      constexpr rend()         { return m_Data.rend(); }
        RangedForConstReverseIteratorType constexpr rend()   const { return m_Data.rend(); }
        // clang-format on
    };

} // namespace worse