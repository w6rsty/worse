#pragma once
#include <cstddef>
#include <utility>

namespace worse::ecs
{

    // =========================================================================
    // Type List
    // =========================================================================

    template <typename... Type> struct TypeList
    {
        using type                  = TypeList;
        static constexpr usize size = sizeof...(Type);
    };

    template <usize, typename> struct TypeListElementAt;

    template <usize Index, typename First, typename... Other>
    struct TypeListElementAt<Index, TypeList<First, Other...>>
        : TypeListElementAt<Index - 1u, TypeList<Other...>>
    {
    };

    template <typename First, typename... Other>
    struct TypeListElementAt<0u, TypeList<First, Other...>>
    {
        using type = First;
    };

    template <usize Index, typename List>
    using TypeListElementAt_t = typename TypeListElementAt<Index, List>::type;

    template <usize Index, typename List>
        requires(Index < List::size)
    struct TypeListRemoveAt
    {
    private:
        template <usize... I1, usize... I2>
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

    template <usize Index, typename List>
    using TypeListRemoveAt_t = TypeListElementAt<Index, List>;

    /**
     * @brief Add N for every element in Seq
     */
    template <usize N, usize... Seq>
    constexpr std::index_sequence<N + Seq...>
    sequenceAdd(std::index_sequence<Seq...>)
    {
        return {};
    }

    /**
     * @brief Index range type from Min to Max
     */
    template <usize Min, usize Max>
    using makeIndexRange =
        decltype(sequenceAdd<Min>(std::make_index_sequence<Max - Min>()));

} // namespace worse::ecs