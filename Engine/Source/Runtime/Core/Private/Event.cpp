#include "Event.hpp"

#include <array>
#include <vector>

namespace worse
{
    namespace
    {
        std::array<std::vector<EventSubscribeFn>, k_eventTypeCount> s_subscribers;
    }

    void EventBus::subscribe(EventType const type, EventSubscribeFn&& fn)
    {
        s_subscribers[static_cast<std::size_t>(type)].emplace_back(std::move(fn));
    }

    void EventBus::fire(EventType const type, Event const& payload)
    {
        for (auto& subscriber : s_subscribers[static_cast<std::size_t>(type)])
        {
            subscriber(payload);
        }
    }

} // namespace worse