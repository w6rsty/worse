#pragma once
#include "base_type.hpp"

#include <utility>
#include <vector>

namespace Worse::ecs
{
    // Template-based type erasure using CRTP pattern
    struct ResourceBase
    {
        virtual ~ResourceBase() = default;
    };

    // Template wrapper that stores the resource directly - much simpler
    template <typename T>
    class ResourceWrapper : public ResourceBase
    {
    public:
        template <typename... Args>
        ResourceWrapper(Args&&... args) : resource(std::forward<Args>(args)...)
        {
        }

        T resource;
    };

    // Resource handle for type-safe access
    template <typename T>
    class Resource
    {
    public:
        explicit Resource(T* ptr) : m_ptr(ptr)
        {
        }

        T* operator->() const
        {
            return m_ptr;
        }

        T& operator*() const
        {
            return *m_ptr;
        }

        T* get() const
        {
            return m_ptr;
        }

        operator Bool() const
        {
            return m_ptr != nullptr;
        }

    private:
        T* m_ptr;
    };

    struct ResourceArrayBase
    {
        virtual ~ResourceArrayBase() = default;
    };

    // ResourceArrayWrapper that stores the actual data
    template <typename T>
    class ResourceArrayWrapper : public ResourceArrayBase
    {
    public:
        ResourceArrayWrapper() = default;

        template <typename... Args>
        Size add(Args&&... args)
        {
            resources.emplace_back(std::forward<Args>(args)...);
            return resources.size() - 1;
        }

        Size add(T&& resource)
        {
            resources.push_back(std::move(resource));
            return resources.size() - 1;
        }

        Size add(T const& resource)
        {
            resources.push_back(resource);
            return resources.size() - 1;
        }

        Bool remove(Size const index)
        {
            if (index >= resources.size())
            {
                return false;
            }
            resources.erase(resources.begin() + index);
            return true;
        }

        T const* get(Size const index) const
        {
            if (index >= resources.size())
            {
                return nullptr;
            }
            return &resources[index];
        }

        T* get(Size const index)
        {
            if (index >= resources.size())
            {
                return nullptr;
            }
            return &resources[index];
        }

        Size size() const
        {
            return resources.size();
        }

        Bool empty() const
        {
            return resources.empty();
        }

        void clear()
        {
            resources.clear();
        }

        std::vector<T>& data()
        {
            return resources;
        }

        std::vector<T> const& data() const
        {
            return resources;
        }

        std::vector<T> resources;
    };

    // ResourceArray handle for type-safe access (similar to Resource)
    template <typename T>
    class ResourceArray
    {
    public:
        explicit ResourceArray(ResourceArrayWrapper<T>* ptr) : m_ptr(ptr)
        {
        }

        template <typename... Args>
        Size add(Args&&... args)
        {
            return m_ptr ? m_ptr->add(std::forward<Args>(args)...) : 0;
        }

        Size add(T&& resource)
        {
            return m_ptr ? m_ptr->add(std::move(resource)) : 0;
        }

        Size add(T const& resource)
        {
            return m_ptr ? m_ptr->add(resource) : 0;
        }

        Bool remove(Size const index)
        {
            return m_ptr ? m_ptr->remove(index) : false;
        }

        T const* get(Size const index) const
        {
            return m_ptr ? m_ptr->get(index) : nullptr;
        }

        T* get(Size const index)
        {
            return m_ptr ? m_ptr->get(index) : nullptr;
        }

        Size size() const
        {
            return m_ptr ? m_ptr->size() : 0;
        }

        Bool empty() const
        {
            return m_ptr ? m_ptr->empty() : true;
        }

        void clear()
        {
            if (m_ptr)
            {
                m_ptr->clear();
            }
        }

        ResourceArrayWrapper<T>* operator->() const
        {
            return m_ptr;
        }

        ResourceArrayWrapper<T>& operator*() const
        {
            return *m_ptr;
        }

        ResourceArrayWrapper<T>* getWrapper() const
        {
            return m_ptr;
        }

        operator Bool() const
        {
            return m_ptr != nullptr;
        }

    private:
        ResourceArrayWrapper<T>* m_ptr;
    };

} // namespace Worse::ecs