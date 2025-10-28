#include "Common.hlsl"

Texture2D<float4> input : register(t0, space1);
RWTexture2D<float4> output : register(u0, space1);

float3 threshold(float3 color)
{
    float const BLOOM_THRESHOLD = 1.0;
    float const BLOOM_SOFT_KNEE = 0.5;
    float const MAX_BRIGHTNESS  = 60.0;

    color               = min(color, MAX_BRIGHTNESS);
    float brightness    = dot(color, float3(0.2126, 0.7152, 0.0722)); // luminance for accuracy
    float soft          = brightness - BLOOM_THRESHOLD + BLOOM_SOFT_KNEE;
    soft                = clamp(soft, 0, 2 * BLOOM_SOFT_KNEE);
    soft                = soft * soft / (4 * BLOOM_SOFT_KNEE + FLT_MIN);
    float contribution  = max(soft, brightness - BLOOM_THRESHOLD);
    contribution       /= max(brightness, FLT_MIN);
    color               = color * contribution;

    // Karis average for firefly reduction
    float luma          = dot(color, float3(0.2126, 0.7152, 0.0722));
    color               /= (1.0 + luma);

    return color;
}

[numthreads(THREAD_GROUP_COUNT_X, THREAD_GROUP_COUNT_Y, 1)]
void main_cs(uint3 threadID : SV_DispatchThreadID)
{
    float2 resolution;
    output.GetDimensions(resolution.x, resolution.y);

    if (any(threadID.xy >= resolution))
    {
        return;
    }

    float4 uvScaleOffset = getF4();

    float2 uv = (threadID.xy + 0.5) / resolution;
    float3 color = input.SampleLevel(samplers[samplerBilinearWrap], uv, 0).rgb;
    float3 filtered = threshold(color);

    output[threadID.xy] = float4(saturate_16(filtered), 1.0);
}
