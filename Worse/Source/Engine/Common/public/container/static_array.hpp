#pragma once
#include "base_type.hpp"
#include "common_macro.hpp"
#include "container/container_define.hpp"

#include <utility>
#include <type_traits>

namespace Worse
{

    namespace Common::Detail
    {
        template <typename InElementType, typename... ArgTypes>
        constexpr Bool CanBeConvertedToFromAll_V = (std::is_convertible_v<ArgTypes, InElementType> && ...);
    }

    /* Array with a static number of elements */
    template <typename InElementType, UInt NumElements, UInt Alignment = alignof(InElementType)>
    class alignas(Alignment) TStaticArray
    {
    public:
        using ElementType = InElementType;

        constexpr TStaticArray() = default;

        // Constructs each element with args
        template <typename... ArgTypes>
        constexpr explicit TStaticArray(EInPlace, ArgTypes&&... args)
            : m_storage(InPlace, std::make_integer_sequence<UInt, NumElements>(), std::forward<ArgTypes>(args)...)
        {
        }

        // Initializes array with provided values
        template <
            typename... ArgTypes,
            typename = std::enable_if_t<(sizeof...(ArgTypes) > 0 && sizeof...(ArgTypes) <= NumElements) && Common::Detail::CanBeConvertedToFromAll_V<InElementType, ArgTypes...>>>
        constexpr TStaticArray(ArgTypes&&... args)
            : m_storage(PerElement, std::forward<ArgTypes>(args)...)
        {
        }

        constexpr TStaticArray(TStaticArray const&)            = default;
        constexpr TStaticArray(TStaticArray&&)                 = default;
        constexpr TStaticArray& operator=(TStaticArray const&) = default;
        constexpr TStaticArray& operator=(TStaticArray&&)      = default;

        // Accessors
        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr InElementType& operator[](UInt index)
        {
            return m_storage.elements[index].element;
        }

        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr InElementType const& operator[](UInt index) const
        {
            return m_storage.elements[index].element;
        }

        // Comparisons
        WORSE_NODISCARD constexpr friend Bool operator==(TStaticArray const& lhs, TStaticArray const& rhs)
        {
            for (UInt elementIndex = 0; elementIndex < NumElements; ++elementIndex)
            {
                if (!(lhs[elementIndex] == rhs[elementIndex]))
                {
                    return kFalse;
                }
            }
            return kTrue;
        }

        WORSE_NODISCARD constexpr Bool operator!=(TStaticArray const& other) const
        {
            for (UInt elementIndex = 0; elementIndex < NumElements; ++elementIndex)
            {
                if (!((*this)[elementIndex] == other[elementIndex]))
                {
                    return kTrue;
                }
            }
            return kFalse;
        }

        WORSE_NODISCARD constexpr Bool IsEmpty() const
        {
            return NumElements == 0;
        }

        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr Int Num() const
        {
            return s_cast<Int>(NumElements);
        }

        WORSE_NODISCARD WORSE_FORCE_INLINE constexpr InElementType* GetData()
        {
            static_assert((alignof(ElementType) % Alignment) == 0, "GetData() cannot be called on a TStaticArray with no-standard alignment");
            return &m_storage.elements[0].element;
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
            constexpr explicit ArrayStorageElementAligned(EInPlace, UInt /*index*/, ArgTypes&&... args)
                : element(std::forward<ArgTypes>(args)...)
            {
            }

            InElementType element;
        };

        struct ArrayStorage
        {
            constexpr ArrayStorage() = default;

            template <UInt... Indices, typename... ArgTypes>
            constexpr explicit ArrayStorage(EInPlace, std::integer_sequence<UInt, Indices...>, ArgTypes... args)
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

        ArrayStorage m_storage;

    public:
        /* Constness is defined by StorageElemebtType */
        template <typename StorageElementType, Bool bReverse = kFalse>
        class RangedForIterator
        {
        public:
            constexpr explicit RangedForIterator(StorageElementType* ptr)
                : m_ptr(ptr)
            {
            }

            WORSE_NODISCARD constexpr auto& operator*() const
            {
                if constexpr (bReverse)
                {
                    return (m_ptr - 1)->element;
                }
                else
                {
                    return m_ptr->element;
                }
            }

            constexpr RangedForIterator& operator++()
            {
                if constexpr (bReverse)
                {
                    --m_ptr;
                }
                else
                {
                    ++m_ptr;
                }
                return *this;
            }

            WORSE_NODISCARD constexpr Bool operator!=(RangedForIterator const& other) const
            {
                return m_ptr != other.m_ptr;
            }

        private:
            StorageElementType* m_ptr;
        };

        using RangedForIteratorType             = RangedForIterator<ArrayStorageElementAligned>;
        using RangedForConstIteratorType        = RangedForIterator<ArrayStorageElementAligned const>;
        using RangedForReverseIteratorType      = RangedForIterator<ArrayStorageElementAligned, kTrue>;
        using RangedForConstReverseIteratorType = RangedForIterator<ArrayStorageElementAligned const, kTrue>;

        // clang-format off
        RangedForIteratorType             constexpr begin()        { return RangedForIteratorType(m_storage.elements); }
        RangedForConstIteratorType        constexpr begin()  const { return RangedForConstIteratorType(m_storage.elements); }
        RangedForIteratorType             constexpr end()          { return RangedForIteratorType(m_storage.elements + NumElements); }
        RangedForConstIteratorType        constexpr end()    const { return RangedForConstIteratorType(m_storage.elements + NumElements); }
        RangedForReverseIteratorType      constexpr rbegin()       { return RangedForReverseIteratorType(m_storage.elements + NumElements); }
        RangedForConstReverseIteratorType constexpr rbegin() const { return RangedForConstReverseIteratorType(m_storage.elements + NumElements); }
        RangedForReverseIteratorType      constexpr rend()         { return RangedForReverseIteratorType(m_storage.elements); }
        RangedForConstReverseIteratorType constexpr rend()   const { return RangedForConstReverseIteratorType(m_storage.elements); }
        // clang-format on
    };

} // namespace Worse
