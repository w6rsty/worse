#pragma once
#include "Math/Math.hpp"
#include "RHIResource.hpp"

namespace worse
{

    WS_DEFINE_FLAGS(RHIBufferUsage, std::uint8_t);
    struct RHIBufferUsageFlagBits
    {
        // clang-format off
        static constexpr RHIBufferUsageFlags None    {0b0000'0000};
        static constexpr RHIBufferUsageFlags Vertex  {0b0001'0001};
        static constexpr RHIBufferUsageFlags Instance{0b0001'0010};
        static constexpr RHIBufferUsageFlags Index   {0b0001'0100};
        static constexpr RHIBufferUsageFlags Storage {0b0010'0000};
        static constexpr RHIBufferUsageFlags Uniform {0b1000'0000};
        // clang-format on
    };

    class RHIBuffer : public RHIResource
    {
        void nativeCreate(void const* data);
        void nativeDestroy();

    public:
        RHIBuffer() = default;
        RHIBuffer(RHIBufferUsageFlags const usage, std::uint32_t const stride,
                  std::uint32_t const elementCount, void const* data,
                  bool const mappable = false, std::string_view name = "Buffer")
        {
            m_usage        = usage;
            m_stride       = stride;
            m_elementCount = elementCount;
            m_size         = m_stride * m_elementCount;
            m_mappable     = mappable;

            nativeCreate(data);
        }

        ~RHIBuffer()
        {
            nativeDestroy();
        }

        // update mapped buffer data
        void update(RHICommandList* cmdList, void const* cpuData,
                    std::uint32_t const size);
        void resetOffset()
        {
            m_offset      = 0;
            m_firstUpdate = true;
        }

        // clang-format off
        RHIBufferUsageFlags getUsage() const         { return m_usage; }
        std::uint32_t       getStride() const       { return m_stride; }
        std::uint32_t       getOffset() const       { return m_offset; }
        std::uint32_t       getElementCount() const { return m_elementCount; }
        std::uint32_t       getSize() const         { return m_size; }
        void*               getMappedData() const   { return m_gpuData; }
        RHINativeHandle     getHandle() const       { return m_handle; }
        // clang-format on
    private:
        RHIBufferUsageFlags m_usage  = RHIBufferUsageFlagBits::None;
        std::uint32_t m_stride       = 0;
        std::uint32_t m_offset       = 0;
        std::uint32_t m_elementCount = 0;
        std::uint32_t m_size         = 0;
        void* m_gpuData              = nullptr;
        bool m_mappable              = false;
        bool m_firstUpdate           = true;

        RHINativeHandle m_handle = {};
    };

} // namespace worse