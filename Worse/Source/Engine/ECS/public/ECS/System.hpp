#pragma once
#include "TypeList.hpp"
#include "Commands.hpp"
#include "QueryView.hpp"
#include "EventBus.hpp"
#include "Resource.hpp"
#include "Registry.hpp"

#include <functional>

namespace worse::ecs
{

    // =========================================================================
    // Funciton traits
    // =========================================================================

    template <typename> struct FunctionPointerTraits;

    /**
     * @brief Free function specialization
     */
    template <typename Ret, typename... Args>
    struct FunctionPointerTraits<Ret(Args...)>
    {
        using type = Ret(Args...);
    };

    /**
     * @brief Function pointer specialization
     */
    template <typename Ret, typename... Args>
    struct FunctionPointerTraits<Ret (*)(Args...)>
    {
        using type = Ret(Args...);
    };

    template <typename> struct SystemTraits;

    template <typename... Args> struct SystemTraits<void(Args...)>
    {
        using arg_list = TypeList<Args...>;
    };

    namespace detail
    {
        // =====================================================================
        // Commands traits
        // =====================================================================

        template <typename> struct IsCommands
        {
            static constexpr bool value = false;
        };

        template <> struct IsCommands<Commands>
        {
            static constexpr bool value = true;
        };

        inline Commands constructCommands(Registry& registry)
        {
            return Commands{registry};
        }

        // =====================================================================
        // QuerView traits
        // =====================================================================

        template <typename> struct IsQueryView
        {
            static constexpr bool value = false;
        };

        template <typename... Components>
        struct IsQueryView<QueryView<Components...>>
        {
            static constexpr bool value = true;
        };

        template <typename TypeList> struct ExpandTypeList;

        template <typename... Components>
        struct ExpandTypeList<TypeList<Components...>>
        {
            static auto query(Registry& registry)
            {
                return registry.query<Components...>();
            }
        };

        template <typename Type> auto constructQueryView(Registry& registry)
        {
            using ComponentTypes =
                typename QueryViewTraits<Type>::ComponentTypes;
            return ExpandTypeList<ComponentTypes>::query(registry);
        }

        // =====================================================================
        // EventReader traits
        // =====================================================================

        template <typename> struct IsEventReader
        {
            static constexpr bool value = false;
        };

        template <typename Event>
        struct IsEventReader<std::shared_ptr<EventReader<Event>>>
        {
            static constexpr bool value = true;
        };

        template <typename T> struct EventReaderTraits;

        template <typename Event>
        struct EventReaderTraits<std::shared_ptr<EventReader<Event>>>
        {
            using EventType = Event;
        };

        template <typename Type>
        inline auto constructEventReader(Registry& registry)
        {
            return registry
                .getEventReader<typename EventReaderTraits<Type>::EventType>();
        }

        // =====================================================================
        // Resource traits
        // =====================================================================

        template <typename> struct IsResource
        {
            static constexpr bool value = false;
        };

        template <typename Type> struct IsResource<Resource<Type>>
        {
            static constexpr bool value = true;
        };

        template <typename T> struct ResourceTraits;

        template <typename Type> struct ResourceTraits<Resource<Type>>
        {
            using ResourceType = Type;
        };

        template <typename Type>
        inline auto constructResource(Registry& registry)
        {
            using ResourceType = typename ResourceTraits<Type>::ResourceType;
            return registry.getResource<ResourceType>();
        }

        // =====================================================================
        // ResourceArray traits
        // =====================================================================

        template <typename> struct IsResourceArray
        {
            static constexpr bool value = false;
        };

        template <typename Type> struct IsResourceArray<ResourceArray<Type>>
        {
            static constexpr bool value = true;
        };

        template <typename T> struct ResourceArrayTraits;

        template <typename Type> struct ResourceArrayTraits<ResourceArray<Type>>
        {
            using ResourceType = Type;
        };

        template <typename Type>
        inline auto constructResourceArray(Registry& registry)
        {
            using ResourceType =
                typename ResourceArrayTraits<Type>::ResourceType;
            return registry.getResourceArray<ResourceType>();
        }
        // =====================================================================
        // System wrapper
        // =====================================================================

        template <typename Type>
        [[nodiscard]] auto ResolveParameter(Registry& registry)
        {
            if constexpr (IsCommands<Type>::value)
            {
                return constructCommands(registry);
            }
            else if constexpr (IsQueryView<Type>::value)
            {
                return constructQueryView<Type>(registry);
            }
            else if constexpr (IsEventReader<Type>::value)
            {
                return constructEventReader<Type>(registry);
            }
            else if constexpr (IsResource<Type>::value)
            {
                return constructResource<Type>(registry);
            }
            else if constexpr (IsResourceArray<Type>::value)
            {
                return constructResourceArray<Type>(registry);
            }
            else
            {
                // try using a default constructor, this may failed
                return Type{};
            }
        }

    } // namespace detail

    /**
     * @brief Convert functions to unified form and assgin parameters
     */
    class SystemWrapper
    {
        /**
         * @brief Assign ECS data for given parameters
         */
        template <auto Func, typename ParamList, usize... Idx>
        static constexpr void
        invokeWithResolvedParameters(Registry& registry,
                                     std::index_sequence<Idx...>) noexcept
        {
            std::invoke(
                Func,
                detail::ResolveParameter<TypeListElementAt_t<Idx, ParamList>>(
                    registry)...);
        }

    public:
        using FunctionType = void (*)(Registry&);

        template <auto Func>
        [[nodiscard]] static constexpr FunctionType wrap() noexcept
        {
            using param_list =
                typename SystemTraits<typename FunctionPointerTraits<
                    std::decay_t<decltype(Func)>>::type>::arg_list;
            return [](Registry& registry)
            {
                invokeWithResolvedParameters<Func, param_list>(
                    registry,
                    makeIndexRange<0, param_list::size>{});
            };
        }
    };

} // namespace worse::ecs