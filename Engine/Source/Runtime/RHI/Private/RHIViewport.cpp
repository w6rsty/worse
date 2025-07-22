#include "RHIViewport.hpp"

namespace worse
{
    RHIViewport::RHIViewport(f32 const x, f32 const y, f32 const width,
                             f32 const height, f32 const depthMin,
                             f32 const depthMax)
        : x(x), y(y), width(width), height(height), depthMin(depthMin),
          depthMax(depthMax)
    {
    }

    bool RHIViewport::operator==(RHIViewport const& rhs) const
    {
        return (x == rhs.x) && (y == rhs.y) && (width == rhs.width) &&
               (height == rhs.height) && (depthMin == rhs.depthMin) &&
               (depthMax == rhs.depthMax);
    }

    bool RHIViewport::operator!=(RHIViewport const& rhs) const
    {
        return !(*this == rhs);
    }

    bool RHIViewport::isValid() const
    {
        return (width != 0.0f) && (height != 0.0f);
    }

    f32 RHIViewport::getAspectRatio() const
    {
        return width / height;
    }

} // namespace worse