#include "Common.hlsl"

Texture2D input0 : register(t0, space1);
Texture2D input1 : register(t1, space1);
Texture2D input2 : register(t2, space1);
Texture2D input3 : register(t3, space1);
RWTexture2D<float4> output : register(u0, space1);

float3 upscaleFilter(Texture2D<float4> src, float2 uv, float2 texelSize, float bloomSpread)
{
    // 3x3 Gussian kernel
    float3 c0 = src.SampleLevel(samplers[samplerBilinearWrap], uv + texelSize * float2(-1.0, -1.0) * bloomSpread, 0).rgb * (1.0 / 16.0);
    float3 c1 = src.SampleLevel(samplers[samplerBilinearWrap], uv + texelSize * float2(-1.0,  1.0) * bloomSpread, 0).rgb * (1.0 / 16.0);
    float3 c2 = src.SampleLevel(samplers[samplerBilinearWrap], uv + texelSize * float2( 1.0, -1.0) * bloomSpread, 0).rgb * (1.0 / 16.0);
    float3 c3 = src.SampleLevel(samplers[samplerBilinearWrap], uv + texelSize * float2( 1.0,  1.0) * bloomSpread, 0).rgb * (1.0 / 16.0);
    float3 c4 = src.SampleLevel(samplers[samplerBilinearWrap], uv + texelSize * float2(-1.0,  0.0) * bloomSpread, 0).rgb * (2.0 / 16.0);
    float3 c5 = src.SampleLevel(samplers[samplerBilinearWrap], uv + texelSize * float2( 1.0,  0.0) * bloomSpread, 0).rgb * (2.0 / 16.0);
    float3 c6 = src.SampleLevel(samplers[samplerBilinearWrap], uv + texelSize * float2( 0.0, -1.0) * bloomSpread, 0).rgb * (2.0 / 16.0);
    float3 c7 = src.SampleLevel(samplers[samplerBilinearWrap], uv + texelSize * float2( 0.0,  1.0) * bloomSpread, 0).rgb * (2.0 / 16.0);
    float3 c8 = src.SampleLevel(samplers[samplerBilinearWrap], uv + texelSize * float2( 0.0,  0.0) * bloomSpread, 0).rgb * (4.0 / 16.0);

    return c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8;
}

// upscale and additive blend
[numthreads(THREAD_GROUP_COUNT_X, THREAD_GROUP_COUNT_Y, 1)]
void main_cs(uint3 threadID : SV_DispatchThreadID)
{
    float2 resolution;
    output.GetDimensions(resolution.x, resolution.y);

    if (any(threadID.xy >= resolution))
    {
        return;
    }

    float2 uv = (threadID.xy + 0.5) / resolution;
    float2 texelSize = 1.0 / resolution;

    float3 sample0 = upscaleFilter(input0, uv, texelSize, 1.0);
    float3 sample1 = upscaleFilter(input1, uv, texelSize, 2.0);
    float3 sample2 = upscaleFilter(input2, uv, texelSize, 3.0);
    float3 sample3 = upscaleFilter(input3, uv, texelSize, 4.0);

    float3 bloom = (sample0 + sample1 + sample2 + sample3) * 0.25;

    output[threadID.xy] = float4(bloom, 1.0);
}