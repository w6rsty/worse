#pragma once
#include "BaseTypes.hpp"

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
    static constexpr Size k_eventTypeCount = static_cast<Size>(EventType::Max);

    using Event            = std::variant<std::monostate, int, void*>;
    using EventSubscribeFn = std::function<void(Event const&)>;

    class EventBus
    {
    public:
        static void subscribe(EventType const type, EventSubscribeFn&& fn);
        static void fire(EventType const type, Event const& payload = std::monostate{});
    };
} // namespace worse