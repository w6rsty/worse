#pragma once
#include <cstddef>
#include <utility>

namespace Worse::ecs
{

    // =========================================================================
    // Type List
    // =========================================================================

    template <typename... Type>
    struct TypeList
    {
        using type                  = TypeList;
        static constexpr Size size = sizeof...(Type);
    };

    template <Size, typename>
    struct TypeListElementAt;

    template <Size Index, typename First, typename... Other>
    struct TypeListElementAt<Index, TypeList<First, Other...>>
        : TypeListElementAt<Index - 1u, TypeList<Other...>>
    {
    };

    template <typename First, typename... Other>
    struct TypeListElementAt<0u, TypeList<First, Other...>>
    {
        using type = First;
    };

    template <Size Index, typename List>
    using TypeListElementAt_t = typename TypeListElementAt<Index, List>::type;

    template <Size Index, typename List>
        requires(Index < List::size)
    struct TypeListRemoveAt
    {
    private:
        template <Size... I1, Size... I2>
        static auto helper(std::index_sequence<I1...>,
                           std::index_sequence<I2...>)
        {
            return TypeList<TypeListElementAt<I1, List>...,
                            TypeListElementAt_t<I2 + Index + 1, List>...>{};
        }

    public:
        using type = decltype(helper(
            std::make_index_sequence<Index>{},
            std::make_index_sequence<List::size - Index - 1>{}));
    };

    template <Size Index, typename List>
    using TypeListRemoveAt_t = TypeListElementAt<Index, List>;

    /**
     * @brief Add N for every element in Seq
     */
    template <Size N, Size... Seq>
    constexpr std::index_sequence<N + Seq...>
    sequenceAdd(std::index_sequence<Seq...>)
    {
        return {};
    }

    /**
     * @brief Index range type from Min to Max
     */
    template <Size Min, Size Max>
    using makeIndexRange =
        decltype(sequenceAdd<Min>(std::make_index_sequence<Max - Min>()));

} // namespace Worse::ecs