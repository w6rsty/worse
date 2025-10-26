#include "Common.hlsl"
#include "Utils.hlsl"

// =============================================================================
// Begin Toggles
// =============================================================================

// #define FILTER_KUWAHARA
static int const KUWAHARA_RADIUS = 3;

#define FILTER_BLOOM

#define FILTER_ACES

// #define FILTER_DITHERING

#define FILTER_GAMMA_CORRECTION
static float const GAMMA = 2.2;

// #define FILTER_VIGNETTE
static float const VIGNETTE_START = 0.3;
static float const VIGNETTE_END = 0.9;

// =============================================================================
// End Toggles
// =============================================================================

Texture2D<float4> input0 : register(t0, space1);
Texture2D<float4> input1 : register(t1, space1);
RWTexture2D<float4> output : register(u0, space1);

[numthreads(THREAD_GROUP_COUNT_X, THREAD_GROUP_COUNT_Y, 1)]
void main_cs(uint3 threadID : SV_DispatchThreadID)
{
    uint2 resolution;
    input0.GetDimensions(resolution.x, resolution.y);

    if (any(threadID.xy >= resolution))
    {
        return;
    }

    float3 finalColor = input0[threadID.xy].rgb;

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
                int2 offset = threadID.xy + regionOffsets[r] + int2(x, y);
                offset = clamp(offset, int2(0, 0), resolution - 1);

                float4 color = input0[offset];
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
#endif FILTER_KUWAHARA

#ifdef FILTER_BLOOM
    float2 uvBloom = (float2(threadID.xy) + 0.5) / float2(resolution);
    float3 bloomColor = input1.SampleLevel(samplers[samplerBilinearWrap], uvBloom, 0).rgb;

    finalColor += bloomColor;
#endif FILTER_BLOOM

#ifdef FILTER_ACES
    finalColor = ACESFilm(finalColor);
#endif FILTER_ACES

#ifdef FILTER_DITHERING
    float2 ditherCoord = float2(threadID.xy);
    float dither = frac(sin(dot(ditherCoord, float2(12.9898, 78.233))) * 43758.5453);
    dither = (dither - 0.5) / 255.0; // Scale to 8-bit precision
    finalColor += dither;
#endif FILTER_DITHERING

#ifdef FILTER_GAMMA_CORRECTION
      finalColor = pow(finalColor, 1.0 / GAMMA);
#endif FILTER_GAMMA_CORRECTION

#ifdef FILTER_VIGNETTE
    float2 uv = float2(threadID.xy) / float2(resolution);
    float2 centered = uv - 0.5;
    float vignette = 1.0 - smoothstep(VIGNETTE_START, VIGNETTE_END, length(centered));
    finalColor *= vignette;
#endif FILTER_VIGNETTE

    output[threadID.xy] = float4(finalColor, 1.0);
}