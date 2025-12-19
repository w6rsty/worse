#pragma once

#include "BaseTypes.hpp"
#include "Macro/Common.hpp"
#include "Container/ContainerDefines.hpp"

#include <utility>
#include <type_traits>

namespace worse
{

    namespace Common::Detail
    {
        template <typename InElementType, typename... ArgTypes>
        constexpr bool CanBeConvertedToFromAll_V = (std::is_convertible_v<ArgTypes, InElementType> && ...);
    }

    /* Array with a static number of elements */
    template <typename InElementType, U32 NumElements, U32 Alignment = alignof(InElementType)>
    class alignas(Alignment) TStaticArray
    {
    public:
        using ElementType = InElementType;

        constexpr TStaticArray() = default;

        // Constructs each element with args
        template <typename... ArgTypes>
        constexpr explicit TStaticArray(EInPlace, ArgTypes&&... args)
            : m_Storage(InPlace, std::make_integer_sequence<U32, NumElements>(), std::forward<ArgTypes>(args)...)
        {
        }

        // Initializes array with provided values
        template <
            typename... ArgTypes,
            typename = std::enable_if_t<(sizeof...(ArgTypes) > 0 && sizeof...(ArgTypes) <= NumElements) && Common::Detail::CanBeConvertedToFromAll_V<InElementType, ArgTypes...>>>
        constexpr TStaticArray(ArgTypes&&... args)
            : m_Storage(PerElement, std::forward<ArgTypes>(args)...)
        {
        }

        constexpr TStaticArray(TStaticArray const&)            = default;
        constexpr TStaticArray(TStaticArray&&)                 = default;
        constexpr TStaticArray& operator=(TStaticArray const&) = default;
        constexpr TStaticArray& operator=(TStaticArray&&)      = default;

        // Accessors
        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr InElementType& operator[](U32 index)
        {
            return m_Storage.elements[index].element;
        }

        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr InElementType const& operator[](U32 index) const
        {
            return m_Storage.elements[index].element;
        }

        // Comparisons
        WORSE_NODISCARD constexpr friend bool operator==(TStaticArray const& lhs, TStaticArray const& rhs)
        {
            for (U32 elementIndex = 0; elementIndex < NumElements; ++elementIndex)
            {
                if (!(lhs[elementIndex] == rhs[elementIndex]))
                {
                    return false;
                }
            }
            return true;
        }

        WORSE_NODISCARD constexpr bool operator!=(TStaticArray const& other) const
        {
            for (U32 elementIndex = 0; elementIndex < NumElements; ++elementIndex)
            {
                if (!((*this)[elementIndex] == other[elementIndex]))
                {
                    return true;
                }
            }
            return false;
        }

        WORSE_NODISCARD constexpr bool IsEmpty() const
        {
            return NumElements == 0;
        }

        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr I32 Num() const
        {
            return s_cast<I32>(NumElements);
        }

        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr InElementType* GetData()
        {
            static_assert((alignof(ElementType) % Alignment) == 0, "GetData() cannot be called on a TStaticArray with no-standard alignment");
            return &m_Storage.elements[0].element;
        }

        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr InElementType const* GetData() const
        {
            return c_cast<TStaticArray*>(this)->GetData();
        }

    private:
        struct alignas(Alignment) ArrayStorageElementAligned
        {
            constexpr ArrayStorageElementAligned() = default;

            template <typename... ArgTypes>
            constexpr explicit ArrayStorageElementAligned(EInPlace, U32 /*index*/, ArgTypes&&... args)
                : element(std::forward<ArgTypes>(args)...)
            {
            }

            InElementType element;
        };

        struct ArrayStorage
        {
            constexpr ArrayStorage() = default;

            template <U32... Indices, typename... ArgTypes>
            constexpr explicit ArrayStorage(EInPlace, std::integer_sequence<U32, Indices...>, ArgTypes... args)
                : elements{ArrayStorageElementAligned(InPlace, Indices, args...)...}
            {
                // Deliberately use non-forwarded arguments to trigger compile error when receiving
                // a move-only object, to avoid multiple times of ownership transfer
            }

            template <typename... ArgTypes>
            constexpr explicit ArrayStorage(EPerElement, ArgTypes&&... args)
                : elements{ArrayStorageElementAligned(InPlace, 0, std::forward<ArgTypes>(args))...}
            {
            }

            ArrayStorageElementAligned elements[NumElements];
        };

        ArrayStorage m_Storage;

    public:
        /* Constness is defined by StorageElemebtType */
        template <typename StorageElementType, bool bReverse = false>
        class RangedForIterator
        {
        public:
            constexpr explicit RangedForIterator(StorageElementType* ptr)
                : m_Ptr(ptr)
            {
            }

            WORSE_NODISCARD constexpr auto& operator*() const
            {
                if constexpr (bReverse)
                {
                    return (m_Ptr - 1)->element;
                }
                else
                {
                    return m_Ptr->element;
                }
            }

            constexpr RangedForIterator& operator++()
            {
                if constexpr (bReverse)
                {
                    --m_Ptr;
                }
                else
                {
                    ++m_Ptr;
                }
                return *this;
            }

            WORSE_NODISCARD constexpr bool operator!=(RangedForIterator const& other) const
            {
                return m_Ptr != other.m_Ptr;
            }

        private:
            StorageElementType* m_Ptr;
        };

        using RangedForIteratorType             = RangedForIterator<ArrayStorageElementAligned>;
        using RangedForConstIteratorType        = RangedForIterator<ArrayStorageElementAligned const>;
        using RangedForReverseIteratorType      = RangedForIterator<ArrayStorageElementAligned, true>;
        using RangedForConstReverseIteratorType = RangedForIterator<ArrayStorageElementAligned const, true>;

        // clang-format off
        RangedForIteratorType             constexpr begin()        { return RangedForIteratorType(m_Storage.elements); }
        RangedForConstIteratorType        constexpr begin()  const { return RangedForConstIteratorType(m_Storage.elements); }
        RangedForIteratorType             constexpr end()          { return RangedForIteratorType(m_Storage.elements + NumElements); }
        RangedForConstIteratorType        constexpr end()    const { return RangedForConstIteratorType(m_Storage.elements + NumElements); }
        RangedForReverseIteratorType      constexpr rbegin()       { return RangedForReverseIteratorType(m_Storage.elements + NumElements); }
        RangedForConstReverseIteratorType constexpr rbegin() const { return RangedForConstReverseIteratorType(m_Storage.elements + NumElements); }
        RangedForReverseIteratorType      constexpr rend()         { return RangedForReverseIteratorType(m_Storage.elements); }
        RangedForConstReverseIteratorType constexpr rend()   const { return RangedForConstReverseIteratorType(m_Storage.elements); }
        // clang-format on
    };

} // namespace worse
