#pragma once
#include "TypeList.hpp"
#include "Commands.hpp"
#include "QueryView.hpp"
#include "EventBus.hpp"
#include "Resource.hpp"
#include "Registry.hpp"

#include <functional>
#include <type_traits>

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
        // QueryView traits
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
                // 尝试默认构造
                static_assert(
                    std::is_default_constructible_v<Type>,
                    "Type must be default constructible or a valid ECS type.");
                return Type{};
            }
        }

    } // namespace detail

    class SystemWrapper
    {
        // clang-format off
        /**
         * @brief 依据参数类型从 Registry 中获取实参后调用原始函数
         */
        template <auto Func, typename ParamList, std::size_t... Idx>
        static constexpr void
        invokeWithResolvedParameters(Registry& registry, std::index_sequence<Idx...>) noexcept
        {
            std::invoke(
                Func,
                detail::ResolveParameter<TypeListElementAt_t<Idx, ParamList>>(registry)...
            );
        }

    public:
        struct Function
        {
            using Type = void (*)(Registry&);

            Type ptr;
            std::uintptr_t index;

            void operator()(Registry& registry) const
            {
                ptr(registry);
            }
        };

        /**
         * @brief 将函数包装为统一的 Function 类型
         */
        template <auto Func>
        [[nodiscard]] static Function wrap() noexcept
        {
            // 1. 获取 Func 退化类型
            using decayedFuncType = std::decay_t<decltype(Func)>;
            // 2. 转化为标准函数指针类型
            using functionPtrType = typename FunctionPointerTraits<decayedFuncType>::type;
            // 3. 获取参数列表
            using paramList = typename SystemTraits<functionPtrType>::arg_list;

            return Function{
                .ptr = [](Registry& registry)
                    {
                        // 解析参数，通过 Registry 获取实参
                        invokeWithResolvedParameters<Func, paramList>(
                            registry,
                            // 构建 index sequence 来遍历参数列表
                            makeIndexRange<0, paramList::size>{}
                        );
                    },
                // 保存类型
                .index = reinterpret_cast<std::uintptr_t>(Func)};
        }

        // clang-format on
    };

} // namespace worse::ecs