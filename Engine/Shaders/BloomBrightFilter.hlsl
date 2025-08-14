#include "Common.hlsl"

Texture2D<float4> input : register(t0, space1);
RWTexture2D<float4> output : register(u0, space1);

[numthreads(THREAD_GROUP_COUNT_X, THREAD_GROUP_COUNT_Y, 1)]
void main_cs(uint3 id : SV_DispatchThreadID)
{
    uint width, height;
    output.GetDimensions(width, height);

    if (id.x >= width || id.y >= height)
    {
        return;
    }

    float4 uvScaleOffset = getF4();
    float bloomThreshold = getF2().x;
    float intensity = getF2().y;

    float2 uv = (float2(id.xy) + 0.5) / float2(width, height);

    // 四个采样坐标
    float4 uvOffset1 = uvScaleOffset.xyxy * float4(-1.0, -1.0, 1.0, -1.0) + float4(uv.x, uv.y, uv.x, uv.y);
    float4 uvOffset2 = uvScaleOffset.xyxy * float4(-1.0,  1.0, 1.0,  1.0) + float4(uv.x, uv.y, uv.x, uv.y);

    float3 sample00 = input.SampleLevel(samplers[samplerBilinearWrap], uvOffset1.xy, 0).rgb;
    float3 sample01 = input.SampleLevel(samplers[samplerBilinearWrap], uvOffset1.zw, 0).rgb;
    float3 sample10 = input.SampleLevel(samplers[samplerBilinearWrap], uvOffset2.xy, 0).rgb;
    float3 sample11 = input.SampleLevel(samplers[samplerBilinearWrap], uvOffset2.zw, 0).rgb;

    float3 average = (sample00 + sample01 + sample10 + sample11) * 0.25;

    // 亮度限制
    float luminance = dot(average, float3(0.2126, 0.7152, 0.0722));
    float bloomFactor = smoothstep(bloomThreshold, bloomThreshold + 0.1, luminance);
    average *= bloomFactor * intensity;

    output[id.xy] = float4(average, 1.0);
}
