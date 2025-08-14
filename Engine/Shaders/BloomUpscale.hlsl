#include "Common.hlsl"

Texture2D<float4> input0 : register(t0, space1);
Texture2D<float4> input1 : register(t1, space1);
Texture2D<float4> input2 : register(t2, space1);
Texture2D<float4> input3 : register(t3, space1);
RWTexture2D<float4> output : register(u0, space1);

// upscale and additive blend
[numthreads(THREAD_GROUP_COUNT_X, THREAD_GROUP_COUNT_Y, 1)] void
main_cs(uint3 id : SV_DispatchThreadID)
{
    uint width, height;
    output.GetDimensions(width, height);

    if (id.x >= width || id.y >= height)
    {
          return;
    }

    float2 uv = (float2(id.xy) + 0.5) / float2(width, height);

    float3 sample0 = input0.SampleLevel(samplers[samplerBilinearWrap], uv, 0).rgb;
    float3 sample1 = input1.SampleLevel(samplers[samplerBilinearWrap], uv, 0).rgb;
    float3 sample2 = input2.SampleLevel(samplers[samplerBilinearWrap], uv, 0).rgb;
    float3 sample3 = input3.SampleLevel(samplers[samplerBilinearWrap], uv, 0).rgb;

    float3 bloom = (sample0 + sample1 + sample2 + sample3) * 0.25;

    output[id.xy] = float4(bloom, 1.0);
}