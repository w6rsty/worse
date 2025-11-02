#include "RHIViewport.hpp"

namespace Worse
{
    RHIViewport::RHIViewport(Float const x, Float const y, Float const width,
                             Float const height, Float const depthMin,
                             Float const depthMax)
        : x(x), y(y), width(width), height(height), depthMin(depthMin),
          depthMax(depthMax)
    {
    }

    Bool RHIViewport::operator==(RHIViewport const& rhs) const
    {
        return (x == rhs.x) && (y == rhs.y) && (width == rhs.width) &&
               (height == rhs.height) && (depthMin == rhs.depthMin) &&
               (depthMax == rhs.depthMax);
    }

    Bool RHIViewport::operator!=(RHIViewport const& rhs) const
    {
        return !(*this == rhs);
    }

    Bool RHIViewport::isValid() const
    {
        return (width != 0.0f) && (height != 0.0f);
    }

    Float RHIViewport::getAspectRatio() const
    {
        return width / height;
    }

} // namespace Worse