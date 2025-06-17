#pragma once
#include "RHIResource.hpp"

#include <array>
#include <vector>

namespace worse
{

    class VulkanDescriptorAllocator
    {
        VkDescriptorPool createPool();
        RHINativeHandle allocateInternal(VkDescriptorSetLayout layout,
                                         void* pNext);

    public:
        VulkanDescriptorAllocator();
        ~VulkanDescriptorAllocator();

        void resetAll();

        RHINativeHandle allocateSet(RHINativeHandle layout);
        RHINativeHandle allocateVariableSet(RHINativeHandle layout,
                                            std::uint32_t count);

    private:
        std::uint32_t m_expandRatio = 1;
        std::uint32_t m_rotateIndex = 0;
        std::array<std::vector<VkDescriptorPool>, 2> m_pools;
        std::array<std::uint32_t, 2> m_currentPoolIndex;
    };

} // namespace worse