#pragma once

#include "BaseTypes.hpp"
#include "Macro/Common.hpp"

#include <atomic>
#include <type_traits>

namespace worse
{

    /**
     * @brief Intrusive counter.
     */
    template <typename T>
    class TRefCounter : public T
    {
    private:
        std::atomic<U32> m_ReferenceCount = 0;

    public:
        virtual U32 IncreaseReference() override
        {
            return ++m_ReferenceCount;
        }

        virtual U32 DecreaseReference() override
        {
            U32 result = --m_ReferenceCount;
            if (result == 0)
            {
                delete this;
            }
            return result;
        }

        virtual U32 GetReferenceCount() override
        {
            return m_ReferenceCount.load();
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
        T* m_Ptr;

        template <typename U>
        friend class TRefCountPtr;

        void InternalIncreaseReference()
        {
            if (m_Ptr != nullptr)
            {
                m_Ptr->IncreaseReference();
            }
        }

        U32 InternalDecreaseReference()
        {
            U32 referenceCount = 0;
            T* ptr             = m_Ptr;

            if (ptr)
            {
                m_Ptr          = nullptr;
                referenceCount = ptr->DecreaseReference();
            }

            return referenceCount;
        }

    public:
        TRefCountPtr() noexcept
            : m_Ptr(nullptr)
        {
        }

        TRefCountPtr(std::nullptr_t) noexcept
            : m_Ptr(nullptr)
        {
        }

        template <typename U>
        TRefCountPtr(U* other) noexcept
            : m_Ptr(other)
        {
            InternalIncreaseReference();
        }

        TRefCountPtr(TRefCountPtr const& other) noexcept
            : m_Ptr(other.m_Ptr)
        {
            InternalIncreaseReference();
        }

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<T*, U*>>>
        TRefCountPtr(TRefCountPtr<U> const& other) noexcept
            : m_Ptr(static_cast<U*>(other.m_ptr))
        {
            InternalIncreaseReference();
        }

        TRefCountPtr(TRefCountPtr&& other) noexcept
            : m_Ptr(nullptr)
        {
            if (this != reinterpret_cast<TRefCountPtr*>(&other))
            {
                Swap(other);
            }
        }

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<T*, U*>>>
        TRefCountPtr(TRefCountPtr<U>&& other) noexcept
            : m_Ptr(other.m_ptr)
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
            if (m_Ptr != other)
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
            if (m_Ptr != other.m_Ptr)
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
            T* tmp      = m_Ptr;
            m_Ptr       = other.m_Ptr;
            other.m_Ptr = tmp;
        }

        void Swap(TRefCountPtr&& other) noexcept
        {
            T* tmp      = m_Ptr;
            m_Ptr       = other.m_Ptr;
            other.m_Ptr = tmp;
        }

        WORSE_NODISCARD T* Get() const noexcept
        {
            return m_Ptr;
        }

        operator T*() const
        {
            return m_Ptr;
        }

        T* operator->() const noexcept
        {
            return m_Ptr;
        }

        T** operator&()
        {
            return &m_Ptr;
        }

        WORSE_NODISCARD T* const* GetAddressOf() const noexcept
        {
            return &m_Ptr;
        }

        WORSE_NODISCARD T** GetAddressOf() noexcept
        {
            return &m_Ptr;
        }

        T* Detach() noexcept
        {
            T* ptr = m_Ptr;
            m_Ptr  = nullptr;
            return ptr;
        }

        void Attach(T* other)
        {
            if (m_Ptr != nullptr)
            {
                U32 referenceCount = m_Ptr->DecreaseReference();
                WORSE_ASSERT(referenceCount != 0 || m_Ptr == other);
            }
            m_Ptr = other;
        }

        static TRefCountPtr<T> Create(T* other)
        {
            TRefCountPtr<T> ptr;
            ptr.Attach(other);
            return ptr;
        }

        U32 Reset()
        {
            return InternalDecreaseReference();
        }

        U32 DbgGetReferenceCount() const noexcept
        {
            U32 referenceCount = 0;
            if (m_Ptr)
            {
                referenceCount = m_Ptr->GetReferenceCount();
            }
            return referenceCount;
        }
    };

} // namespace worse
