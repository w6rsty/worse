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
    output.position = float4(input.position, 1.0);
    output.uv = input.uv;
    output.normal = normalize(input.normal);
    output.tangent = normalize(input.tangent);

    return output;
}

struct PixelOutput
{
    float4 color : SV_Target0;
};

PixelOutput main_ps(VertexOutput input)
{
    PixelOutput output;

    float4 sampledColor = float4(0.0, 0.0, 0.0, 1.0);
    
    float2 centeredUV = input.uv - 0.5;
    float tilt = centeredUV.x + centeredUV.y * 0.5;
    
    if (tilt < 0.0)
    {
        sampledColor = materialTextures[0].Sample(samplers[samplerPointClampEdge], input.uv);
    }
    else
    {
        sampledColor = materialTextures[1].Sample(samplers[samplerPointClampEdge], input.uv);
    }

    output.color = sampledColor;

    return output;
}