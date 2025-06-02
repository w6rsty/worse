#include "RHIViewport.hpp"

namespace worse
{
    RHIViewport::RHIViewport(
        float const x, float const y,
        float const width, float const height,
        float const depthMin, float const depthMax)
        : x(x), y(y), width(width), height(height), depthMin(depthMin), depthMax(depthMax)
    {
    }

    bool RHIViewport::operator==(RHIViewport const& rhs) const
    {
        return (x == rhs.x) && (y == rhs.y) &&
               (width == rhs.width) && (height == rhs.height) &&
               (depthMin == rhs.depthMin) && (depthMax == rhs.depthMax);
    }

    bool RHIViewport::operator!=(RHIViewport const& rhs) const
    {
        return !(*this == rhs);
    }

    bool RHIViewport::isValid() const
    {
        return (width != 0.0f) && (height != 0.0f);
    }

    float RHIViewport::getAspectRatio() const
    {
        return width / height;
    }

} // namespace worse