#include "Common.hlsl"

struct VertexOutput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
    float3 normal   : NORMAL;
    float3 tangent  : TANGENT;
};

VertexOutput main_vs(VertexPosUvNrmTan input)
{
    VertexOutput output;

    float4x4 mvp = mul(frameData.projection, mul(frameData.view, pushData.transform));
    output.position = mul(mvp, float4(input.position, 1.0));

    output.uv = input.uv;
    output.normal = normalize(mul(input.normal, (float3x3)pushData.transform));
    output.tangent = normalize(mul(input.tangent, (float3x3)pushData.transform));

    return output;
}

float random(float2 st)
{
    return frac(sin(dot(st, float2(12.9898, 78.233))) * 43758.5453123);
}

float noise(float2 st)
{
    float2  i = floor(st);
    float2  f = frac (st);

    float a = random(i);
    float b = random(i + float2(1.0, 0.0));
    float c = random(i + float2(0.0, 1.0));
    float d = random(i + float2(1.0, 1.0));

    float2 u = f * f * (3.0 - 2.0 * f);
    return lerp(a, b, u.x) +
           (c - a) * u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y;
}

static const int OCTAVES = 4;
float fbm(float2 st)
{
    float value     = 0.0;
    float amplitude = 0.5;

    [unroll]
    for (int i = 0; i < OCTAVES; ++i)
    {
        value     += amplitude * noise(st);
        st        *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

float3 Shade(float2 uv, float t)
{
    // 1) UV 扭曲 & 内部能量纹理 (应用于整个UV范围)
    float2 distortion_uv  = uv * 3.0;
    float2 distortion_off = float2(fbm(distortion_uv + t),
                                   fbm(distortion_uv - t * 1.2));
    float2 distorted_uv   = uv + (distortion_off * 2.0 - 1.0) * 0.2;

    float innerPattern = fbm(distorted_uv * 6.0 + t * 2.0);
          innerPattern = pow(innerPattern, 3.0) * 2.0;

    // 2) 边缘扰动作为额外的变化
    float edgeNoise = fbm(uv * 5.0 + t) * 0.3;
    innerPattern += edgeNoise;

    // 3) 颜色 (不再使用遮罩，直接应用于整个范围)
    float3 baseColor   = float3(0.1, 0.1, 0.1);
    float3 hotColor    = float3(0.2, 0.8, 0.7);
    float3 shadowColor = baseColor + innerPattern * hotColor;

    // 4) 背景渐变保持，但与主效果混合
    float3 bgColor = float3(0.1, 0.1, 0.1) * (1.0 - length(uv) * 0.3);

    return shadowColor + bgColor;
}

float4 main_ps(VertexOutput pin) : SV_Target
{
    float iTime = frameData.time;
    // 使用输入的 UV 坐标，转换到 [-1, 1] 范围以保持效果一致
    float2 uv = (pin.uv - 0.5) * 2.0;
    float  t  = iTime * 0.3;

    float3 finalColor = Shade(uv, t);

    // 输出线性颜色空间的结果，后处理阶段将处理HDR和色调映射
    return float4(finalColor, 1.0);
}