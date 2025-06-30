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

    return output;
}

void main_ps(VertexOutput input)
{
    // No operations are needed here for the depth prepass.
}