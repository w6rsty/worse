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

Texture2D<float4> materialTextures[] : register(t0, space1);

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
    
    float2 uv = input.uv * 2.0 - 1.0;
    float d = length(uv);
    float theta = atan2(uv.y, uv.x);

    static const float PI = 3.14159265358979323846;

    float sector = floor(theta / (PI / (4 * frameData.time)));
    float phaseShift = fmod(sector, 2.0);

    float wave = sin(PI * (8.0 * d + phaseShift));
    
    float3 white = materialTextures[1].Sample(samplers[samplerBilinearClampEdge], input.uv).rgb;
    float3 black = materialTextures[0].Sample(samplers[samplerBilinearClampEdge], input.uv).rgb;

    float3 color = lerp(black, white, wave > 0.0 ? 1.0 : 0.0);

    output.color = float4(color, 1.0);

    return output;
}