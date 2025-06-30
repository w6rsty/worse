#pragma once
#include "Definitions.hpp"
#include "Storage.hpp"
#include "EventBus.hpp"
#include "Resource.hpp"
#include "QueryView.hpp"

#include <tuple>
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace worse::ecs
{
    // clang-format off

    class Registry
    {
        template <typename Component> Storage<Component>& getOrCreateStorage()
        {
            std::type_index typeIndex(typeid(Component));
            auto it = m_storages.find(typeIndex);
            if (it != m_storages.end())
            {
                return static_cast<StorageWrapper<Component>*>(it->second.get())->storage;
            }
            else
            {
                auto wrapper = std::make_unique<StorageWrapper<Component>>();
                Storage<Component>* ptr = &wrapper->storage;
                m_storages[typeIndex]   = std::move(wrapper);
                return *ptr;
            }
        }

        template <typename ResourceType>
        ResourceWrapper<ResourceType>* getResourceWrapper()
        {
            std::type_index typeIndex(typeid(ResourceType));
            auto it = m_resources.find(typeIndex);
            if (it != m_resources.end())
            {
                return static_cast<ResourceWrapper<ResourceType>*>(it->second.get());
            }
            return nullptr;
        }

    public:
        Registry() = default;

        Registry(Registry const&)            = delete;
        Registry& operator=(Registry const&) = delete;

        ~Registry()
        {
            // Clean up all storages
            for (auto& [typeIndex, storage] : m_storages)
            {
                storage.reset();
            }
            m_storages.clear();
            
            // Clean up all resources
            for (auto& [typeIndex, resource] : m_resources)
            {
                resource.reset();
            }
            m_resources.clear();
            
            m_entities.clear();
        }

        Entity create()
        {
            return m_entities.generate();
        }

        void destroy(Entity entity)
        {
            m_entities.remove(entity);
            for (auto& [typeIndex, storage] : m_storages)
            {
                // Remove the entity from all storages
                storage->remove(entity);
            }
        }

        template <typename Component, typename... Args>
            requires(!std::is_empty_v<Component>)
        Component& addComponent(Entity entity, Args&&... args)
        {
            return getOrCreateStorage<Component>().emplace(entity, std::forward<Args>(args)...);
        }

        template <typename Component>
            requires(std::is_empty_v<Component>)
        void addComponent(Entity entity)
        {
            getOrCreateStorage<Component>().emplace(entity);
        }

        // Unified addComponent interface that handles both empty and non-empty
        // types
        template <typename Component>
        auto addComponent(Entity entity, Component&& component) -> decltype(auto)
        {
            using DecayedComponent = std::decay_t<Component>;
            if constexpr (std::is_empty_v<DecayedComponent>)
            {
                addComponent<DecayedComponent>(entity);
            }
            else
            {
                return getOrCreateStorage<DecayedComponent>().emplace(entity, std::forward<Component>(component));
            }
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
            auto storages = std::forward_as_tuple(getOrCreateStorage<Components>()...);
            return QueryView<Components...>(*this, m_entities, storages);
        }

        template <typename Event>
        void emitEvent(Event&& event, EventPriority const priority = EventPriority::Normal)
        {
            m_eventBus.send(std::move(event), priority);
        }

        template <typename Event>

        void emitEventImmediate(Event&& event, EventPriority const priority = EventPriority::Normal)
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

        template <typename Type, typename... Args>
        Type& emplaceResource(Args&&... args)
        {
            std::type_index typeIndex(typeid(Type));
            auto wrapper = std::make_unique<ResourceWrapper<Type>>(std::forward<Args>(args)...);
            Type* ptr = &wrapper->resource;
            m_resources[typeIndex] = std::move(wrapper);
            return *ptr;
        }

        template <typename Type> 
        void removeResource()
        {
            std::type_index typeIndex(typeid(Type));
            m_resources.erase(typeIndex);
        }

        template <typename Type>
        Resource<Type> getResource()
        {
            auto* wrapper = getResourceWrapper<Type>();
            WS_ASSERT(wrapper);
            return wrapper ? Resource<Type>(&wrapper->resource) : Resource<Type>(nullptr);
        }

        template <typename Type>
        bool hasResource()
        {
            return getResourceWrapper<Type>() != nullptr;
        }

        template <typename Type>
        ResourceArray<Type> emplaceResourceArray()
        {
            std::type_index typeIndex(typeid(Type));
            auto wrapper = std::make_unique<ResourceArrayWrapper<Type>>();
            ResourceArray<Type> ptr(wrapper.get());
            m_resourceArrays[typeIndex] = std::move(wrapper);
            return ptr;
        }

        template <typename Type>
        void removeResourceArray()
        {
            std::type_index typeIndex(typeid(Type));
            m_resourceArrays.erase(typeIndex);
        }

        template <typename Type>
        ResourceArray<Type> getResourceArray()
        {
            std::type_index typeIndex(typeid(Type));
            auto it = m_resourceArrays.find(typeIndex);
            if (it != m_resourceArrays.end())
            {
                return ResourceArray<Type>(static_cast<ResourceArrayWrapper<Type>*>(it->second.get()));
            }
            return ResourceArray<Type>(nullptr);
        }   

        template <typename Type>
        bool hasResourceArray()
        {
            std::type_index typeIndex(typeid(Type));
            return m_resourceArrays.find(typeIndex) != m_resourceArrays.end();
        }

    private:
        Storage<Entity> m_entities;
        std::unordered_map<std::type_index, std::unique_ptr<StorageBase>> m_storages;
        EventBus m_eventBus;
        std::unordered_map<std::type_index, std::unique_ptr<ResourceBase>> m_resources;
        std::unordered_map<std::type_index, std::unique_ptr<ResourceArrayBase>> m_resourceArrays;
    };

    // clang-format on
} // namespace worse::ecs
