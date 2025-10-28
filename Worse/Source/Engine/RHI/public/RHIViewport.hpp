#pragma once
#include "Types.hpp"

namespace worse
{

    struct RHIViewport
    {
        RHIViewport(f32 const x = 0.0f, f32 const y = 0.0f,
                    f32 const width = 0.0f, f32 const height = 0.0f,
                    f32 const depthMin = 0.0f, f32 const depthMax = 1.0f);
        RHIViewport(RHIViewport const&) = default;
        ~RHIViewport()                  = default;

        bool operator==(RHIViewport const& rhs) const;
        bool operator!=(RHIViewport const& rhs) const;
        bool isValid() const;
        f32 getAspectRatio() const;

        f32 x        = 0.0f;
        f32 y        = 0.0f;
        f32 width    = 0.0f;
        f32 height   = 0.0f;
        f32 depthMin = 0.0f;
        f32 depthMax = 0.0f;

        static const RHIViewport undefined;
    };

} // namespace worse