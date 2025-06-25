#pragma once
#include "Storage.hpp"
#include "EventBus.hpp"
#include "QueryView.hpp"

#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <memory>

namespace worse::ecs
{
    class World
    {
        struct StorageBase
        {
            virtual ~StorageBase() = default;
        };

        template <typename T> struct StorageWrapper : public StorageBase
        {
            Storage<T> storage;
        };

        template <typename Component> Storage<Component>& getOrCreateStorage()
        {
            std::type_index typeIndex(typeid(Component));
            auto it = m_storages.find(typeIndex);
            if (it != m_storages.end())
            {
                return static_cast<StorageWrapper<Component>*>(it->second.get())
                    ->storage;
            }
            else
            {
                auto wrapper = std::make_unique<StorageWrapper<Component>>();
                Storage<Component>* ptr = &wrapper->storage;
                m_storages[typeIndex]   = std::move(wrapper);
                return *ptr;
            }
        }

    public:
        World() = default;

        World(World const&)            = delete;
        World& operator=(World const&) = delete;

        ~World()
        {
            // Clean up all storages
            for (auto& [typeIndex, storage] : m_storages)
            {
                storage.reset();
            }
            m_storages.clear();
            m_entities.clear();
        }

        Entity create()
        {
            return m_entities.generate();
        }

        template <typename Component, typename... Args>
            requires(!std::is_empty_v<Component>)
        Component& addComponent(Entity entity, Args&&... args)
        {
            return getOrCreateStorage<Component>().emplace(
                entity,
                std::forward<Args>(args)...);
        }

        template <typename Component>
            requires(std::is_empty_v<Component>)
        void addComponent(Entity entity)
        {
            getOrCreateStorage<Component>().emplace(entity);
        }

        template <typename Component> Component& getComponent(Entity entity)
        {
            return getOrCreateStorage<Component>().get(entity);
        }

        template <typename Component> bool hasComponent(Entity entity)
        {
            return getOrCreateStorage<Component>().contains(entity);
        }

        // pack component storage pools
        template <typename... Components> QueryView<Components...> query()
        {
            auto storages =
                std::forward_as_tuple(getOrCreateStorage<Components>()...);
            return QueryView<Components...>(*this, m_entities, storages);
        }

        template <typename Event>
        void sendEvent(Event&& event,
                       EventPriority const priority = EventPriority::Normal)
        {
            m_eventBus.send(std::move(event), priority);
        }

        template <typename Event>

        void sendEventImmediate(Event&& event, EventPriority const priority =
                                                   EventPriority::Critical)
        {
            m_eventBus.sendImmediate(std::move(event), priority);
        }

        template <typename Event>
        std::shared_ptr<EventReader<Event>> getEventReader()
        {
            return m_eventBus.getReader<Event>();
        }

        void dipatchEvents()
        {
            m_eventBus.dispatch();
        }

    private:
        Storage<Entity> m_entities;
        std::unordered_map<std::type_index, std::unique_ptr<StorageBase>>
            m_storages;

        EventBus m_eventBus;
    };
} // namespace worse::ecs