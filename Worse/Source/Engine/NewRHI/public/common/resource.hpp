#pragma once

#include "base_type.hpp"
#include "macro/common_macro.hpp"

#include <atomic>
#include <type_traits>

namespace Worse
{

    enum class RHIObjectType
    {
        VK_Instance,
        VK_PhysicalDevice,
        VK_Device,
        VK_Queue,
        VK_CommandBuffer,
        VK_DeviceMemory,
        VK_Buffer,
        VK_Image,
        VK_ImageView,
        VK_Sampler,
        VK_ShaderModule,
        VK_DescriptorPool,
        VK_DescriptorSetLayout,
        VK_DescriptorSet,
        VK_PipelineLayout,
        VK_Pipeline
    };

    struct RHIObject
    {
        union
        {
            UPtrInt integer;
            void* pointer;
        };

        RHIObject(UPtrInt i)
            : integer(i)
        {
        }
        RHIObject(void* p)
            : pointer(p)
        {
        }

        template <typename T>
        operator T*() const { return static_cast<T*>(pointer); }
    };

    /**
     * @brief RHI resource interface
     */
    class IResource
    {
    protected:
        IResource()          = default;
        virtual ~IResource() = default;

    public:
        virtual UInt IncreaseReference() = 0;
        virtual UInt DecreaseReference() = 0;
        virtual UInt GetReferenceCount() = 0;

        virtual RHIObject GetNativeObject(RHIObjectType objectType)
        {
            (void)objectType;
            return nullptr;
        }

        IResource(IResource const&)            = delete;
        IResource(IResource&&)                 = delete;
        IResource& operator=(IResource const&) = delete;
        IResource& operator=(IResource&&)      = delete;
    };

    /**
     * @brief Intrusive counter.
     */
    template <typename T>
    class RefCounter : public T
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
    class RefCountPtr
    {
    public:
        using InterfaceType = T;

    protected:
        T* m_ptr;

        template <typename U>
        friend class RefCountPtr;

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
        RefCountPtr() noexcept
            : m_ptr(nullptr)
        {
        }

        RefCountPtr(std::nullptr_t) noexcept
            : m_ptr(nullptr)
        {
        }

        template <typename U>
        RefCountPtr(U* other) noexcept
            : m_ptr(other)
        {
            InternalIncreaseReference();
        }

        RefCountPtr(RefCountPtr const& other) noexcept
            : m_ptr(other.m_ptr)
        {
            InternalIncreaseReference();
        }

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<T*, U*>>>
        RefCountPtr(RefCountPtr<U> const& other) noexcept
            : m_ptr(static_cast<U*>(other.m_ptr))
        {
            InternalIncreaseReference();
        }

        RefCountPtr(RefCountPtr&& other) noexcept
            : m_ptr(nullptr)
        {
            if (this != reinterpret_cast<RefCountPtr*>(&other))
            {
                Swap(other);
            }
        }

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<T*, U*>>>
        RefCountPtr(RefCountPtr<U>&& other) noexcept
            : m_ptr(other.m_ptr)
        {
            other.m_ptr = nullptr;
        }

        ~RefCountPtr()
        {
            InternalDecreaseReference();
        }

        RefCountPtr& operator=(std::nullptr_t) noexcept
        {
            InternalDecreaseReference();
            return *this;
        }

        RefCountPtr& operator=(T* other) noexcept
        {
            if (m_ptr != other)
            {
                RefCountPtr(other).Swap(*this);
            }
            return *this;
        }

        template <typename U>
        RefCountPtr& operator=(U* other) noexcept
        {
            RefCountPtr(other).Swap(*this);
            return *this;
        }

        RefCountPtr& operator=(RefCountPtr const& other) noexcept
        {
            if (m_ptr != other.m_ptr)
            {
                RefCountPtr(other).Swap(*this);
            }
            return *this;
        }

        template <typename U>
        RefCountPtr& operator=(RefCountPtr<U> const& other) noexcept
        {
            RefCountPtr(other).Swap(*this);
            return *this;
        }

        RefCountPtr& operator=(RefCountPtr&& other) noexcept
        {
            RefCountPtr(static_cast<RefCountPtr&&>(other)).Swap(*this);
            return *this;
        }

        template <typename U>
        RefCountPtr& operator=(RefCountPtr<U>&& other) noexcept
        {
            RefCountPtr(static_cast<RefCountPtr<U>&&>(other)).Swap(*this);
            return *this;
        }

        void Swap(RefCountPtr& other) noexcept
        {
            T* tmp      = m_ptr;
            m_ptr       = other.m_ptr;
            other.m_ptr = tmp;
        }

        void Swap(RefCountPtr&& other) noexcept
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

        static RefCountPtr<T> Create(T* other)
        {
            RefCountPtr<T> ptr;
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