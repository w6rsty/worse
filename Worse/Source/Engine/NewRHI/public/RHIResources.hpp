#pragma once

#include "base_type.hpp"

namespace Worse
{

    enum class ERHIObjectType
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

    struct FRHIObject
    {
        union
        {
            UPtrInt integer;
            void* pointer;
        };

        FRHIObject(UPtrInt i)
            : integer(i)
        {
        }
        FRHIObject(void* p)
            : pointer(p)
        {
        }

        template <typename T>
        operator T*() const { return static_cast<T*>(pointer); }
    };

    /**
     * @brief RHI resource interface
     */
    class IRHIResource
    {
    protected:
        IRHIResource()          = default;
        virtual ~IRHIResource() = default;

    public:
        virtual UInt IncreaseReference() = 0;
        virtual UInt DecreaseReference() = 0;
        virtual UInt GetReferenceCount() = 0;

        virtual FRHIObject GetNativeObject(ERHIObjectType objectType)
        {
            (void)objectType;
            return nullptr;
        }

        IRHIResource(IRHIResource const&)            = delete;
        IRHIResource(IRHIResource&&)                 = delete;
        IRHIResource& operator=(IRHIResource const&) = delete;
        IRHIResource& operator=(IRHIResource&&)      = delete;
    };

} // namespace Worse