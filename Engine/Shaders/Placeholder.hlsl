struct VertexPosUvNrmTan
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
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
    float4 color : SV_Target;
};

PixelOutput main_ps(VertexOutput input)
{
    PixelOutput output;
    output.color = float4(1.0, 0.0, 1.0, 1.0);

    return output;
}