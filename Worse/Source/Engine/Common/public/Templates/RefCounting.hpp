#pragma once

#include "base_type.hpp"
#include "macro/common_macro.hpp"

#include <atomic>
#include <type_traits>

namespace Worse
{

    /**
     * @brief Intrusive counter.
     */
    template <typename T>
    class TRefCounter : public T
    {
    private:
        std::atomic<UInt> m_referenceCount = 0;

    public:
        virtual UInt IncreaseReference() override
        {
            return ++m_referenceCount;
        }

        virtual UInt DecreaseReference() override
        {
            UInt result = --m_referenceCount;
            if (result == 0)
            {
                delete this;
            }
            return result;
        }

        virtual UInt GetReferenceCount() override
        {
            return m_referenceCount.load();
        }
    };

    /**
     * @brief Reference-counted pointer.
     */
    template <typename T>
    class TRefCountPtr
    {
    public:
        using InterfaceType = T;

    protected:
        T* m_ptr;

        template <typename U>
        friend class TRefCountPtr;

        void InternalIncreaseReference()
        {
            if (m_ptr != nullptr)
            {
                m_ptr->IncreaseReference();
            }
        }

        UInt InternalDecreaseReference()
        {
            UInt referenceCount = 0;
            T* ptr              = m_ptr;

            if (ptr)
            {
                m_ptr          = nullptr;
                referenceCount = ptr->DecreaseReference();
            }

            return referenceCount;
        }

    public:
        TRefCountPtr() noexcept
            : m_ptr(nullptr)
        {
        }

        TRefCountPtr(std::nullptr_t) noexcept
            : m_ptr(nullptr)
        {
        }

        template <typename U>
        TRefCountPtr(U* other) noexcept
            : m_ptr(other)
        {
            InternalIncreaseReference();
        }

        TRefCountPtr(TRefCountPtr const& other) noexcept
            : m_ptr(other.m_ptr)
        {
            InternalIncreaseReference();
        }

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<T*, U*>>>
        TRefCountPtr(TRefCountPtr<U> const& other) noexcept
            : m_ptr(static_cast<U*>(other.m_ptr))
        {
            InternalIncreaseReference();
        }

        TRefCountPtr(TRefCountPtr&& other) noexcept
            : m_ptr(nullptr)
        {
            if (this != reinterpret_cast<TRefCountPtr*>(&other))
            {
                Swap(other);
            }
        }

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<T*, U*>>>
        TRefCountPtr(TRefCountPtr<U>&& other) noexcept
            : m_ptr(other.m_ptr)
        {
            other.m_ptr = nullptr;
        }

        ~TRefCountPtr()
        {
            InternalDecreaseReference();
        }

        TRefCountPtr& operator=(std::nullptr_t) noexcept
        {
            InternalDecreaseReference();
            return *this;
        }

        TRefCountPtr& operator=(T* other) noexcept
        {
            if (m_ptr != other)
            {
                TRefCountPtr(other).Swap(*this);
            }
            return *this;
        }

        template <typename U>
        TRefCountPtr& operator=(U* other) noexcept
        {
            TRefCountPtr(other).Swap(*this);
            return *this;
        }

        TRefCountPtr& operator=(TRefCountPtr const& other) noexcept
        {
            if (m_ptr != other.m_ptr)
            {
                TRefCountPtr(other).Swap(*this);
            }
            return *this;
        }

        template <typename U>
        TRefCountPtr& operator=(TRefCountPtr<U> const& other) noexcept
        {
            TRefCountPtr(other).Swap(*this);
            return *this;
        }

        TRefCountPtr& operator=(TRefCountPtr&& other) noexcept
        {
            TRefCountPtr(static_cast<TRefCountPtr&&>(other)).Swap(*this);
            return *this;
        }

        template <typename U>
        TRefCountPtr& operator=(TRefCountPtr<U>&& other) noexcept
        {
            TRefCountPtr(static_cast<TRefCountPtr<U>&&>(other)).Swap(*this);
            return *this;
        }

        void Swap(TRefCountPtr& other) noexcept
        {
            T* tmp      = m_ptr;
            m_ptr       = other.m_ptr;
            other.m_ptr = tmp;
        }

        void Swap(TRefCountPtr&& other) noexcept
        {
            T* tmp      = m_ptr;
            m_ptr       = other.m_ptr;
            other.m_ptr = tmp;
        }

        WORSE_NODISCARD T* Get() const noexcept
        {
            return m_ptr;
        }

        operator T*() const
        {
            return m_ptr;
        }

        T* operator->() const noexcept
        {
            return m_ptr;
        }

        T** operator&()
        {
            return &m_ptr;
        }

        WORSE_NODISCARD T* const* GetAddressOf() const noexcept
        {
            return &m_ptr;
        }

        WORSE_NODISCARD T** GetAddressOf() noexcept
        {
            return &m_ptr;
        }

        T* Detach() noexcept
        {
            T* ptr = m_ptr;
            m_ptr  = nullptr;
            return ptr;
        }

        void Attach(T* other)
        {
            if (m_ptr != nullptr)
            {
                UInt referenceCount = m_ptr->DecreaseReference();
                WORSE_ASSERT(referenceCount != 0 || m_ptr == other);
            }
            m_ptr = other;
        }

        static TRefCountPtr<T> Create(T* other)
        {
            TRefCountPtr<T> ptr;
            ptr.Attach(other);
            return ptr;
        }

        UInt Reset()
        {
            return InternalDecreaseReference();
        }

        UInt DbgGetReferenceCount() const noexcept
        {
            UInt referenceCount = 0;
            if (m_ptr)
            {
                referenceCount = m_ptr->GetReferenceCount();
            }
            return referenceCount;
        }
    };

} // namespace Worse
