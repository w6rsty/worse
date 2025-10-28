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
    output.position = mul(mvp,float4(input.position, 1.0));

    output.uv = input.uv;
    output.normal = normalize(mul(input.normal, (float3x3)pushData.transform));

    return output;
}

struct PixelOutput
{
    float4 color : SV_Target;
};

PixelOutput main_ps(VertexOutput input)
{
    PixelOutput output;
    float2 checker = floor(input.uv * 8.0);
    float checkerPattern = fmod(checker.x + checker.y, 2.0);
    float3 result = lerp(float3(0.4, 0.4, 0.4), float3(0.7, 0.7, 0.7), checkerPattern);
    result = lerp(result, input.normal * 0.5 + 0.5, 0.5);

    output.color = float4(result, 1.0);

    return output;
}