#pragma once
#include "container/enum_array.hpp"
#include "RHIDefinitions.hpp"

#include <string>
#include <string_view>
#include <type_traits>

namespace Worse
{

    class RHINativeHandle
    {
    public:
        RHINativeHandle() = default;

        template <typename T>
        RHINativeHandle(T* handle, RHINativeHandleType type)
            : m_handle(static_cast<void*>(handle)), m_type(type)
        {
        }

        RHINativeHandle(RHINativeHandle const&)            = default;
        RHINativeHandle& operator=(RHINativeHandle const&) = default;

        RHINativeHandle(RHINativeHandle&& other) noexcept
            : m_handle(other.m_handle), m_type(other.m_type)
        {
            other.reset();
        }

        RHINativeHandle& operator=(RHINativeHandle&& other) noexcept
        {
            if (&other == this)
            {
                return *this;
            }

            m_handle = other.m_handle;
            m_type   = other.m_type;
            other.reset();

            return *this;
        }

        template <typename T>
        T asValue() const
        {
            using Type = std::remove_cvref_t<T>;
            static_assert(sizeof(Type) == sizeof(void*), "Invalid cast");
            return reinterpret_cast<Type>(m_handle);
        }

        ULong asValue() const
        {
            return asValue<ULong>();
        }

        RHINativeHandleType getType() const
        {
            return m_type;
        }

        void reset()
        {
            m_handle = nullptr;
            m_type   = RHINativeHandleType::Max;
        }

        Bool isValid() const
        {
            return m_handle != nullptr;
        }

        explicit operator Bool() const
        {
            return isValid();
        }

    protected:
        void* m_handle             = nullptr;
        RHINativeHandleType m_type = RHINativeHandleType::Max;
    };

    // Base class for all RHI resources
    class RHIResource
    {
    public:
        RHIResource(std::string_view name = "") : m_name(name)
        {
        }

        std::string getName() const
        {
            return m_name;
        }

    protected:
        std::string m_name;
    };

    // Renderer use this inteface claas to provide basic RHI resources
    class RHIResourceProvider
    {
    public:
        virtual ~RHIResourceProvider() = default;

        // clang-format off
        virtual std::pair<RHIShader*, RHIShader*> getPlaceholderShader() const = 0;
        virtual RHITexture* getPlaceholderTexture() const = 0;
        virtual RHIBuffer* getFrameConstantBuffer() const = 0;
        virtual EnumArray<RHISamplerType, RHISampler*> getSamplers() const = 0;
        // clang-format on

        // make sure all resources are ready
        Bool validate()
        {
            Bool validated = true;
            validated &= ((getPlaceholderShader().first != nullptr) &&
                          (getPlaceholderShader().second != nullptr));
            validated &= (getPlaceholderTexture() != nullptr);
            validated &= (getFrameConstantBuffer() != nullptr);

            return validated;
        }
    };

} // namespace Worse