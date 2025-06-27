#pragma once
#include <utility>

namespace worse::ecs
{
    // Template-based type erasure using CRTP pattern
    class ResourceBase
    {
    public:
        virtual ~ResourceBase() = default;
    };

    // Template wrapper that stores the resource directly - much simpler
    template <typename T> class ResourceWrapper : public ResourceBase
    {
    public:
        T resource;

        template <typename... Args>
        ResourceWrapper(Args&&... args) : resource(std::forward<Args>(args)...)
        {
        }
    };

    // Resource handle for type-safe access
    template <typename T> class Resource
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

        operator bool() const
        {
            return m_ptr != nullptr;
        }

    private:
        T* m_ptr;
    };

} // namespace worse::ecs