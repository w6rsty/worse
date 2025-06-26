#include "Math/BoundingBox.hpp"

namespace worse::math
{

    BoundingBox::BoundingBox()
    {
        m_min = Vector3::ZERO();
        m_max = Vector3::ZERO();
    }

    BoundingBox::BoundingBox(Vector3 const& min, Vector3 const& max)
    {
        m_min = min;
        m_max = max;
    }

    BoundingBox::BoundingBox(std::span<Vector3 const> points)
    {
        m_min = Vector3::MAX();
        m_max = Vector3::MIN();

        for (const auto& point : points)
        {
            m_min = min(m_min, point);
            m_max = max(m_max, point);
        }
    }

    BoundingBox::BoundingBox(std::span<RHIVertexPosUvNrmTan const> vertices)
    {
        m_min = Vector3::MAX();
        m_max = Vector3::MIN();

        for (const auto& vertex : vertices)
        {
            m_min = min(m_min, vertex.position);
            m_max = max(m_max, vertex.position);
        }
    }

} // namespace worse::math