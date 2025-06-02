#pragma once

namespace worse
{

    struct RHIViewport
    {
        RHIViewport() = default;
        RHIViewport(
            float const x = 0.0f, float const y = 0.0f,
            float const width = 0.0f, float const height = 0.0f,
            float const depthMin = 0.0f, float const depthMax = 1.0f);
        RHIViewport(RHIViewport const&) = default;
        ~RHIViewport()                  = default;

        bool operator==(RHIViewport const& rhs) const;
        bool operator!=(RHIViewport const& rhs) const;
        bool isValid() const;
        float getAspectRatio() const;

        float x        = 0.0f;
        float y        = 0.0f;
        float width    = 0.0f;
        float height   = 0.0f;
        float depthMin = 0.0f;
        float depthMax = 0.0f;

        static const RHIViewport undefined;
    };

} // namespace worse