struct FrameConstantData
{
    float deltaTime;
    float time;
    float2 padding0; // align to 16 bytes

    float3 cameraPosition;
    float cameraNear;
    float3 cameraForward;
    float cameraFar;
    float4 padding1; // align to 16 bytes

    float4x4 view;
    float4x4 projection;
    float4x4 viewProjection;
};

cbuffer FrameConstantBuffer : register(b0, space0)
{
    FrameConstantData frameData;
};

SamplerComparisonState samplerComparison  : register(s0, space0);
SamplerState           samplers[8]        : register(s1, space0);

static const uint samplerPointClampEdge;
static const uint samplerPointClampBorder;
static const uint samplerWrap;
static const uint samplerBilinearClampEdge;
static const uint samplerBilinearClampBorder;
static const uint samplerBilinearWrap;
static const uint samplerTrilinearClamp;
static const uint samplerAnisotropicClamp;

Texture2D<float4> materialTextures[] : register(t0, space0);

Texture2D<float4> albedo : register(t0, space1);

struct VertexPosUvNrmTan
{
    float3 position : POSITION;
    float2 uv       : TEXCOORD0;
    float3 normal   : NORMAL;
    float3 tangent  : TANGENT;
};

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
    float4 color : SV_Target;
};

PixelOutput main_ps(VertexOutput input)
{
    PixelOutput output;

    float4 sampledColor = float4(0.0, 0.0, 0.0, 1.0);
    
    float2 centeredUV = input.uv - 0.5;
    float tilt = centeredUV.x + centeredUV.y * 0.5;
    
    if (tilt < 0.0)
    {
        sampledColor = materialTextures[1].Sample(samplers[samplerPointClampEdge], input.uv);
    }
    else
    {
        sampledColor = materialTextures[2].Sample(samplers[samplerPointClampEdge], input.uv);
    }

    output.color = sampledColor;

    return output;
}