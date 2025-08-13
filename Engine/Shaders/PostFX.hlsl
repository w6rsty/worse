#include "Common.hlsl"
#include "Utils.hlsl"

// =============================================================================
// Begin Toggles
// =============================================================================

#define FILTER_KUWAHARA
static int const KUWAHARA_RADIUS = 3;

#define FILTER_ACES

#define FILTER_GAMMA_CORRECTION
static float const GAMMA = 2.0;

#define FILTER_VIGNETTE
static float const VIGNETTE_START = 0.3;
static float const VIGNETTE_END = 0.9;

#define FILTER_DITHERING

// =============================================================================
// End Toggles
// =============================================================================

Texture2D<float4> input : register(t0, space1);
RWTexture2D<float4> output : register(u0, space1);

[numthreads(THREAD_GROUP_COUNT_X, THREAD_GROUP_COUNT_Y, 1)] void
main_cs(uint3 id : SV_DispatchThreadID)
{
    uint2 dims;
    input.GetDimensions(dims.x, dims.y);

    if (id.x >= dims.x || id.y >= dims.y)
    {
        return;
    }

    int2 center = int2(id.xy);

    float3 finalColor = input[center].rgb;

#ifdef FILTER_KUWAHARA
    int2 const regionOffsets[4] = {
        int2(-KUWAHARA_RADIUS, -KUWAHARA_RADIUS), int2(0, -KUWAHARA_RADIUS),
        int2(-KUWAHARA_RADIUS, 0), int2(0, 0)
    };

    float4 bestMean = float4(0, 0, 0, 0);
    float bestVariance = 10000.0f;

    int regionSize = KUWAHARA_RADIUS + 1;

    for (int r = 0; r < 4; ++r)
    {
        float4 mean = float4(0, 0, 0, 0);
        float4 sqSum = float4(0, 0, 0, 0);
        int count = 0;

        for (int y = 0; y < regionSize; ++y)
        {
            for (int x = 0; x < regionSize; ++x)
            {
                int2 offset = center + regionOffsets[r] + int2(x, y);
                offset = clamp(offset, int2(0, 0), dims - 1);

                float4 color = input[offset];
                mean += color;
                sqSum += color * color;
                ++count;
            }
        }

        mean /= count;
        float4 varianceVec = sqSum / count - mean * mean;
        float variance = dot(varianceVec.rgb, float3(1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0));

        if (variance < bestVariance)
        {
            bestVariance = variance;
            bestMean = mean;
        }
    }
    finalColor = bestMean.rgb;
#endif

#ifdef FILTER_ACES
    finalColor = ACESFilm(finalColor);
#endif

#ifdef FILTER_GAMMA_CORRECTION
      finalColor = pow(finalColor, 1.0 / GAMMA);
#endif

#ifdef FILTER_VIGNETTE
    float2 uv = float2(id.xy) / float2(dims);
    float2 centered = uv - 0.5;
    float vignette = 1.0 - smoothstep(VIGNETTE_START, VIGNETTE_END, length(centered));
    finalColor *= vignette;
#endif

#ifdef FILTER_DITHERING
    float2 ditherCoord = float2(id.xy);
    float dither = frac(sin(dot(ditherCoord, float2(12.9898, 78.233))) * 43758.5453);
    dither = (dither - 0.5) / 255.0; // Scale to 8-bit precision
     finalColor += dither;
#endif

    output[id.xy] = float4(finalColor, 1.0);
}