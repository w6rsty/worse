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
        using type                        = TypeList;
        static constexpr std::size_t size = sizeof...(Type);
    };

    template <std::size_t, typename> struct TypeListElementAt;

    template <std::size_t Index, typename First, typename... Other>
    struct TypeListElementAt<Index, TypeList<First, Other...>>
        : TypeListElementAt<Index - 1u, TypeList<Other...>>
    {
    };

    template <typename First, typename... Other>
    struct TypeListElementAt<0u, TypeList<First, Other...>>
    {
        using type = First;
    };

    template <std::size_t Index, typename List>
    using TypeListElementAt_t = typename TypeListElementAt<Index, List>::type;

    template <std::size_t Index, typename List>
        requires(Index < List::size)
    struct TypeListRemoveAt
    {
    private:
        template <std::size_t... I1, std::size_t... I2>
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

    template <std::size_t Index, typename List>
    using TypeListRemoveAt_t = TypeListElementAt<Index, List>;

    /**
     * @brief Add N for every element in Seq
     */
    template <std::size_t N, std::size_t... Seq>
    constexpr std::index_sequence<N + Seq...>
    sequenceAdd(std::index_sequence<Seq...>)
    {
        return {};
    }

    /**
     * @brief Index range type from Min to Max
     */
    template <std::size_t Min, std::size_t Max>
    using makeIndexRange =
        decltype(sequenceAdd<Min>(std::make_index_sequence<Max - Min>()));

} // namespace worse::ecs