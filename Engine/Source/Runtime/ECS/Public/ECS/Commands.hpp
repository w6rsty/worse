#pragma once
#include "ECS/Resource.hpp"
#include "Registry.hpp"

namespace worse::ecs
{

    class Commands
    {
    public:
        Commands(Registry& registry) : m_registry{registry}
        {
        }

        template <typename... Components>
        Entity spawn(Components&&... components)
        {
            Entity const entity = m_registry.create();
            (m_registry.addComponent(entity,
                                     std::forward<Components>(components)),
             ...);
            return entity;
        }

        void destroy(Entity entity)
        {
            m_registry.destroy(entity);
        }

        template <typename Component> bool hasComponent(Entity entity)
        {
            return m_registry.hasComponent<Component>(entity);
        }

        template <typename Component> Component& getComponent(Entity entity)
        {
            return m_registry.getComponent<Component>(entity);
        }

        template <typename Event> Commands& emitEvent(Event&& event)
        {
            m_registry.emitEvent(std::forward<Event>(event));
            return *this;
        }

        template <typename Event> Commands& emitEventImmediate(Event&& event)
        {
            m_registry.emitEventImmediate(std::forward<Event>(event));
            return *this;
        }

        template <typename Type, typename... Args>
        Type& emplaceResource(Args&&... args)
        {
            return m_registry.emplaceResource<Type>(
                std::forward<Args>(args)...);
        }

        template <typename Resource> void removeResource()
        {
            m_registry.removeResource<Resource>();
        }

        template <typename Resource> bool hasResource() const
        {
            return m_registry.hasResource<Resource>();
        }

        template <typename Resource>
        ResourceArray<Resource> emplaceResourceArray()
        {
            return m_registry.emplaceResourceArray<Resource>();
        }

        template <typename Resource> ResourceArray<Resource> getResourceArray()
        {
            return m_registry.getResourceArray<Resource>();
        }

        template <typename Resource> bool hasResourceArray() const
        {
            return m_registry.hasResourceArray<Resource>();
        }

    private:
        Registry& m_registry;
    };

} // namespace worse::ecs