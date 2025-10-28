#pragma once
#include <queue>
#include <mutex>
#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include <cstddef>
#include <typeindex>
#include <algorithm>
#include <functional>
#include <unordered_map>

namespace worse::ecs
{

    // Event priority levels
    enum class EventPriority
    {
        Low      = 0,
        Normal   = 1,
        High     = 2,
        Critical = 3
    };

    // Event wrapper with metadata
    template <typename T> struct Event
    {
        T data;
        EventPriority priority = EventPriority::Normal;
        std::chrono::steady_clock::time_point timestamp;
        std::thread::id sourceThread;

        Event(T&& eventData,
              EventPriority const priority = EventPriority::Normal)
            : data(std::move(eventData)), priority(priority),
              timestamp(std::chrono::steady_clock::now()),
              sourceThread(std::this_thread::get_id())
        {
        }

        Event(T const& eventData,
              EventPriority const priority = EventPriority::Normal)
            : data(eventData), priority(priority),
              timestamp(std::chrono::steady_clock::now()),
              sourceThread(std::this_thread::get_id())
        {
        }
    };

    // Event filter interface
    template <typename T>
    using EventFilter = std::function<bool(Event<T> const&)>;

    // Enhanced EventReader with filtering and statistics
    template <typename T> class EventReader
    {
    public:
        EventReader(std::vector<Event<T>>* eventQueue)
            : m_eventQueue(eventQueue), m_cursor(0), m_eventsRead(0)
        {
        }

        std::vector<Event<T>> read()
        {
            if (!m_eventQueue || m_cursor >= m_eventQueue->size())
            {
                return {};
            }

            std::vector<Event<T>> newEvents;
            newEvents.reserve(m_eventQueue->size() - m_cursor);

            for (usize i = m_cursor; i < m_eventQueue->size(); ++i)
            {
                Event<T> const& event = (*m_eventQueue)[i];

                // Apply filters
                bool passedFilters = true;
                for (EventFilter<T> const& filter : m_filters)
                {
                    if (!filter(event))
                    {
                        passedFilters = false;
                        break;
                    }
                }

                if (passedFilters)
                {
                    newEvents.push_back(event);
                    ++m_eventsRead;
                }
            }

            m_cursor = m_eventQueue->size();
            return newEvents;
        }

        // Read only event data (without metadata)
        std::vector<T> readData()
        {
            std::vector<Event<T>> events = read();
            std::vector<T> data;
            data.reserve(events.size());

            for (Event<T>& event : events)
            {
                data.push_back(std::move(event.data));
            }

            return data;
        }

        void updateQueue(std::vector<Event<T>>* eventQueue)
        {
            m_eventQueue = eventQueue;
            m_cursor     = 0;
        }

        // Add event filter
        void addFilter(EventFilter<T> const filter)
        {
            m_filters.push_back(std::move(filter));
        }

        // Clear all filters
        void clearFilters()
        {
            m_filters.clear();
        }

        // Get statistics
        usize getEventsRead() const
        {
            return m_eventsRead;
        }
        usize getPendingEvents() const
        {
            return m_eventQueue ? (m_eventQueue->size() - m_cursor) : 0;
        }

    private:
        std::vector<Event<T>>* m_eventQueue = nullptr;
        usize m_cursor                      = 0;
        usize m_eventsRead                  = 0;
        std::vector<EventFilter<T>> m_filters;
    };

    // Thread-safe EventBus with enhanced features
    class EventBus
    {
        struct IEventChannel
        {
            virtual ~IEventChannel()              = default;
            virtual void swapBuffer()             = 0;
            virtual usize getPendingCount() const = 0;
            virtual usize getTotalSent() const    = 0;
            virtual void cleanupExpiredReaders()  = 0;
        };

        template <typename T> struct EventChannel : public IEventChannel
        {
            mutable std::mutex mutex;
            std::vector<Event<T>> queues[2];
            std::atomic<int> activeQueueIndex{0};
            std::vector<std::weak_ptr<EventReader<T>>> readers;
            std::atomic<usize> totalEventsSent{0};

            // Priority queue for immediate dispatch
            // clang-format off
        std::priority_queue<
            Event<T>,
            std::vector<Event<T>>,
            std::function<bool(Event<T> const&, Event<T> const&)>
        > immediateQueue{[](Event<T> const& a, Event<T> const& b)
                           {
                               return a.priority < b.priority; // Higher priority first
                           }};
            // clang-format on

            void swapBuffer() override
            {
                std::lock_guard<std::mutex> lock(mutex);

                int currentActive = activeQueueIndex.load();

                // Sort events by priority before notifying readers
                std::vector<Event<T>>& activeQueue = queues[currentActive];
                std::stable_sort(activeQueue.begin(),
                                 activeQueue.end(),
                                 [](const Event<T>& a, const Event<T>& b)
                                 {
                                     return a.priority > b.priority;
                                 });

                // Notify readers
                for (auto it = readers.begin(); it != readers.end();)
                {
                    if (auto reader = it->lock())
                    {
                        reader->updateQueue(&activeQueue);
                        ++it;
                    }
                    else
                    {
                        // Remove expired weak_ptr
                        it = readers.erase(it);
                    }
                }

                // Swap and clear recording queue
                int newActive = 1 - currentActive;
                activeQueueIndex.store(newActive);
                queues[newActive].clear();
            }

            usize getPendingCount() const override
            {
                std::lock_guard<std::mutex> lock(mutex);
                return queues[activeQueueIndex.load()].size();
            }

            usize getTotalSent() const override
            {
                return totalEventsSent.load();
            }

            void cleanupExpiredReaders() override
            {
                std::lock_guard<std::mutex> lock(mutex);
                readers.erase(
                    std::remove_if(readers.begin(),
                                   readers.end(),
                                   [](std::weak_ptr<EventReader<T>> const& weak)
                                   {
                                       return weak.expired();
                                   }),
                    readers.end());
            }
        };

        template <typename T> EventChannel<T>& getChannel()
        {
            std::lock_guard<std::mutex> lock(m_mtxChannels);
            std::type_index typeId = std::type_index(typeid(T));

            if (m_channels.find(typeId) == m_channels.end())
            {
                m_channels[typeId] = std::make_unique<EventChannel<T>>();
            }

            return static_cast<EventChannel<T>&>(*m_channels[typeId]);
        }

    public:
        EventBus()  = default;
        ~EventBus() = default;

        // Disable copy constructor and assignment
        EventBus(EventBus const&)            = delete;
        EventBus& operator=(EventBus const&) = delete;

        // Move operations are implicitly deleted due to mutex member

        // Send event with priority
        template <typename T>
        void send(T&& event,
                  EventPriority const priority = EventPriority::Normal)
        {
            EventChannel<T>& channel = getChannel<T>();
            std::lock_guard<std::mutex> lock(channel.mutex);

            int activeIndex = channel.activeQueueIndex.load();
            channel.queues[activeIndex].emplace_back(std::forward<T>(event),
                                                     priority);
            channel.totalEventsSent.fetch_add(1);
        }

        // Send event immediately (bypass normal dispatch cycle)
        template <typename T>
        void
        sendImmediate(T&& event,
                      EventPriority const priority = EventPriority::Critical)
        {
            EventChannel<T>& channel = getChannel<T>();
            std::lock_guard<std::mutex> lock(channel.mutex);

            Event<T> wrappedEvent(std::forward<T>(event), priority);

            // Add to the current active queue for immediate processing
            int activeIndex = channel.activeQueueIndex.load();
            channel.queues[activeIndex].push_back(wrappedEvent);

            // Sort the active queue by priority
            std::stable_sort(channel.queues[activeIndex].begin(),
                             channel.queues[activeIndex].end(),
                             [](const Event<T>& a, const Event<T>& b)
                             {
                                 return a.priority > b.priority;
                             });

            // Notify all readers immediately by updating their queue pointer
            for (auto it = channel.readers.begin();
                 it != channel.readers.end();)
            {
                if (auto reader = it->lock())
                {
                    reader->updateQueue(&channel.queues[activeIndex]);
                    ++it;
                }
                else
                {
                    it = channel.readers.erase(it);
                }
            }

            channel.totalEventsSent.fetch_add(1);
        }

        template <typename T> std::shared_ptr<EventReader<T>> getReader()
        {
            EventChannel<T>& channel = getChannel<T>();
            std::lock_guard<std::mutex> lock(channel.mutex);

            int inactiveIndex = 1 - channel.activeQueueIndex.load();
            std::shared_ptr<EventReader<T>> reader =
                std::make_shared<EventReader<T>>(
                    &channel.queues[inactiveIndex]);
            channel.readers.push_back(reader);
            return reader;
        }

        void dispatch()
        {
            std::lock_guard<std::mutex> lock(m_mtxChannels);
            for (auto& [typeId, channel] : m_channels)
            {
                channel->swapBuffer();
            }
        }

        // cleanup expired reader weak pointers
        void cleanup()
        {
            std::lock_guard<std::mutex> lock(m_mtxChannels);
            for (auto& [typeId, channel] : m_channels)
            {
                channel->cleanupExpiredReaders();
            }
        }

        // =========================================================================
        // Statistics
        // =========================================================================
        template <typename T> usize getPendingEvents() const
        {
            auto& channel = const_cast<EventBus*>(this)->getChannel<T>();
            return channel.getPendingCount();
        }

        template <typename T> usize getTotalEventsSent() const
        {
            auto& channel = const_cast<EventBus*>(this)->getChannel<T>();
            return channel.getTotalSent();
        }

        // get amount of registered event type
        usize getEventTypeCount() const
        {
            std::lock_guard<std::mutex> lock(m_mtxChannels);
            return m_channels.size();
        }

    private:
        mutable std::mutex m_mtxChannels;
        std::unordered_map<std::type_index, std::unique_ptr<IEventChannel>>
            m_channels;
    };

    // =========================================================================
    // Filter Macros
    // =========================================================================

#define WS_EVENT_FILTER_BY_PRIORITY(minPriority)                               \
    [](auto const& event)                                                      \
    {                                                                          \
        return event.priority >= minPriority;                                  \
    }

#define WS_EVENT_FILTER_BY_TIME(maxAge)                                        \
    [](auto const& event)                                                      \
    {                                                                          \
        auto now = std::chrono::steady_clock::now();                           \
        return (now - event.timestamp) <= maxAge;                              \
    }

#define WS_EVENT_FILTER_BY_THREAD(threadId)                                    \
    [threadId](auto const& event)                                              \
    {                                                                          \
        return event.sourceThread == threadId;                                 \
    }

} // namespace worse::ecs