#include "Common.hlsl"

struct VertexOutput
{
    float4 position : SV_Position;
};

VertexOutput main_vs(VertexPosUvNrmTan input)
{
    VertexOutput output;

    output.position = mul(mul(getMatrix(), pushData.transform), float4(input.position, 1.0));
    
    return output;
}

void main_ps(VertexOutput input)
{
    // No operations
}