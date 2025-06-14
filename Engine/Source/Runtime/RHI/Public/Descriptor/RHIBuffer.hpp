#pragma once
#include "RHIResource.hpp"

namespace worse
{

    enum class RHIBufferType : std::uint8_t
    {
        Vertex,
        Index,
        Instance,
        Storage,
        Constant,
        Max
    };

    class RHIBuffer : public RHIResource
    {
        void nativeCreate(void const* data);
        void nativeDestroy();

    public:
        RHIBuffer() = default;
        RHIBuffer(RHIBufferType const type, std::uint32_t const stride,
                  std::uint32_t const elementCount, void const* data,
                  bool const mappable, std::string_view name)
        {
            m_type         = type;
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

        void update(RHICommandList* cmdList, void const* cpuData,
                    std::uint32_t const size);
        void resetOffset()
        {
            m_offset      = 0;
            m_firstUpdate = true;
        }

        // clang-format off
        RHIBufferType   getType() const         { return m_type; }
        std::uint32_t   getStride() const       { return m_stride; }
        std::uint32_t   getOffset() const       { return m_offset; }
        std::uint32_t   getElementCount() const { return m_elementCount; }
        std::uint32_t   getSize() const         { return m_size; }
        void*           getMappedData() const   { return m_gpuData; }
        RHINativeHandle getHandle() const       { return m_handle; }
        // clang-format on
    private:
        RHIBufferType m_type         = RHIBufferType::Max;
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