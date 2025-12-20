#pragma once
#include "base_type.hpp"

namespace Worse
{

    struct RHIViewport
    {
        RHIViewport(Float const x = 0.0f, Float const y = 0.0f,
                    Float const width = 0.0f, Float const height = 0.0f,
                    Float const depthMin = 0.0f, Float const depthMax = 1.0f);
        RHIViewport(RHIViewport const&) = default;
        ~RHIViewport()                  = default;

        Bool operator==(RHIViewport const& rhs) const;
        Bool operator!=(RHIViewport const& rhs) const;
        Bool isValid() const;
        Float getAspectRatio() const;

        Float x        = 0.0f;
        Float y        = 0.0f;
        Float width    = 0.0f;
        Float height   = 0.0f;
        Float depthMin = 0.0f;
        Float depthMax = 0.0f;

        static const RHIViewport undefined;
    };

} // namespace Worse