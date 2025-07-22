#include "Common.hlsl"

Texture2D<float4> albedoTexture : register(t0, space1);
RWTexture2D<float4> output : register(u0, space1);

float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

[numthreads(THREAD_GROUP_COUNT_X, THREAD_GROUP_COUNT_Y, 1)]
void main_cs(uint3 id : SV_DispatchThreadID)
{
    // int radius = 4;

    uint2 dims;
    albedoTexture.GetDimensions(dims.x, dims.y);

    if (id.x >= dims.x || id.y >= dims.y)
    {
        return;
    }

    int2 center = int2(id.xy);

    // const int2 regionOffsets[4] = {
    //     int2(-radius, -radius),
    //     int2(      0, -radius),
    //     int2(-radius,       0),
    //     int2(      0,       0)
    // };

    // float4 bestMean = float4(0, 0, 0, 0);
    // float bestVariance = 10000.0f;

    // int regionSize = radius + 1;

    // for (int r = 0; r < 4; ++r)
    // {
    //     float4 mean = float4(0, 0, 0, 0);
    //     float4 sqSum = float4(0, 0, 0, 0);
    //     int count = 0;

    //     for (int y = 0; y < regionSize; ++y)
    //     {
    //         for (int x = 0; x < regionSize; ++x)
    //         {
    //             int2 offset = center + regionOffsets[r] + int2(x, y);
    //             offset = clamp(offset, int2(0, 0), dims - 1);

    //             float4 color = albedoTexture[offset];
    //             mean += color;
    //             sqSum += color * color;
    //             ++count;
    //         }
    //     }

    //     mean /= count;
    //     float4 varianceVec = sqSum / count - mean * mean;
    //     float variance = dot(varianceVec.rgb, float3(1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0));

    //     if (variance < bestVariance)
    //     {
    //         bestVariance = variance;
    //         bestMean = mean;
    //     }
    // }

    // float3 filteredColor = ACESFilm(bestMean.rgb);

    // float3 filteredColor = ACESFilm(albedoTexture[center].rgb);
    float3 filteredColor  = albedoTexture[center].rgb;

    // Apply gamma correction
    // filteredColor = pow(filteredColor, 1.0f / 2.2f);

    // Add vignette effect
    // float2 uv = float2(id.xy) / float2(dims);
    // float2 centered = uv - 0.5f;
    // float vignette = 1.0f - smoothstep(0.3f, 0.9f, length(centered));
    // filteredColor *= vignette;

    // Add dithering to prevent color banding
    // float2 ditherCoord = float2(id.xy);
    // float dither = frac(sin(dot(ditherCoord, float2(12.9898, 78.233))) * 43758.5453);
    // dither = (dither - 0.5) / 255.0; // Scale to 8-bit precision
    // filteredColor += dither;

    output[id.xy] = float4(filteredColor, 1.0f);
}