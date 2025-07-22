#pragma once
#include "Vector.hpp"
#include "RHITypes.hpp"

#include <span>

namespace worse::math
{

    class BoundingBox
    {
    public:
        BoundingBox();
        BoundingBox(Vector3 const& min, Vector3 const& max);
        BoundingBox(std::span<Vector3 const> points);

        template <RHIVertexConcept Vertex>
        BoundingBox(std::span<Vertex> vertices)
        {
            m_min = Vector3::MAX();
            m_max = Vector3::MIN();

            for (const auto& vertex : vertices)
            {
                m_min = min(m_min, vertex.position);
                m_max = max(m_max, vertex.position);
            }
        }

        // clang-format off
        Vector3 getCenter() const { return (m_min + m_max) * 0.5f; }
        Vector3 getSize() const   { return m_max - m_min; }
        // half size
        Vector3 getExtent() const { return getSize() * 0.5f; }
        float   getVolume() const { Vector3 size = getSize(); return size.x * size.y * size.z; }

        Vector3 const& getMin() const { return m_min; }
        Vector3 const& getMax() const { return m_max; }
        // clang-format on

    private:
        Vector3 m_min;
        Vector3 m_max;
    };

} // namespace worse::math