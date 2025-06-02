#pragma once
#include "Definitions.hpp"

#include <type_traits>

namespace worse
{
    enum class RHIResourceType
    {
        Fence,
        Semaphore,
        Queue,
        Buffer,
        Image,
        ImageView,
        Sampler,
        Shader,
        Pipeline,
        PipelineLayout,
        DescriptorSet,
        DescriptorSetLayout,
        CommandList,
        CommandPool,

        Swapchain,
        Surface,
        Max,
    };

    static constexpr char const*
    rhiResourceTypeToString(RHIResourceType const type)
    {
        switch (type)
        {
            // clang-format off
        case RHIResourceType::Fence:               return "Fence";
        case RHIResourceType::Semaphore:           return "Semaphore";
        case RHIResourceType::Queue:               return "Queue";
        case RHIResourceType::Buffer:              return "Buffer";
        case RHIResourceType::Image:               return "Image";
        case RHIResourceType::ImageView:           return "ImageView";
        case RHIResourceType::Sampler:             return "Sampler";
        case RHIResourceType::Shader:              return "Shader";
        case RHIResourceType::Pipeline:            return "Pipeline";
        case RHIResourceType::PipelineLayout:      return "PipelineLayout";
        case RHIResourceType::DescriptorSet:       return "DescriptorSet";
        case RHIResourceType::DescriptorSetLayout: return "DescriptorSetLayout";
        case RHIResourceType::CommandList:         return "CommandList";
        case RHIResourceType::CommandPool:         return "CommandPool";
        case RHIResourceType::Swapchain:           return "Swapchain";
        case RHIResourceType::Surface:             return "Surface";
        case RHIResourceType::Max:                 WS_ASSERT(false); return "Invalid";
            // clang-format on
        }
    }

    class RHIResource
    {
    public:
        RHIResource() = default;

        template <typename T>
        RHIResource(T* handle, RHIResourceType type)
            : m_handle(static_cast<void*>(handle)), m_type(type)
        {
        }

        RHIResource(RHIResource const&)            = default;
        RHIResource& operator=(RHIResource const&) = default;

        RHIResource(RHIResource&& other)
            : m_handle(other.m_handle), m_type(other.m_type)
        {
            other.reset();
        }

        RHIResource& operator=(RHIResource&& other)
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

        template <typename T> T* asPtr() const
        {
            return static_cast<std::remove_cvref_t<T>*>(m_handle);
        }

        template <typename T> T asValue() const
        {
            using Type = std::remove_cvref_t<T>;
            static_assert(sizeof(Type) == sizeof(void*), "Invalid cast");
            return reinterpret_cast<Type>(m_handle);
        }

        RHIResourceType getType() const
        {
            return m_type;
        }

        void reset()
        {
            m_handle = nullptr;
            m_type   = RHIResourceType::Max;
        }

        bool isValid() const
        {
            return m_handle != nullptr;
        }

        explicit operator bool() const
        {
            return isValid();
        }

    protected:
        void* m_handle         = nullptr;
        RHIResourceType m_type = RHIResourceType::Max;
    };

} // namespace worse