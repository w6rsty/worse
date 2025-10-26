#pragma once
#include "Types.hpp"

#include <variant>
#include <functional>

namespace worse
{
    enum class EventType
    {
        SDL,
        WindowResized,
        Max
    };
    static constexpr usize k_eventTypeCount =
        static_cast<usize>(EventType::Max);

    using Event            = std::variant<std::monostate, int, void*>;
    using EventSubscribeFn = std::function<void(Event const&)>;

    class EventBus
    {
    public:
        static void subscribe(EventType const type, EventSubscribeFn&& fn);
        static void fire(EventType const type,
                         Event const& payload = std::monostate{});
    };
} // namespace worse