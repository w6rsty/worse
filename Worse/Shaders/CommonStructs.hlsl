#ifndef COMMON_STRUCTS_HLSL
#define COMMON_STRUCTS_HLSL

struct SurfaceParameters
{
    uint2 positionScreen;
    float2 uv;
};

struct Surface
{
    float3 albedo;
    float  alpha;
    float  metallic;
    float  roughness;
    float  occlusion;
    float  emission;

    uint2  positionScreen;
    float2 uv;
    float  depth;
    float3 normal;
    float3 positionWorld;
    float3 cameraToPixelDirection;
    float  cameraToPixelDistance;

    void Build(
        SurfaceParameters params,
        Texture2D<float4> gbufferAlbedo,
        Texture2D<float4> gbufferNormal,
        Texture2D<float4> gbufferMaterial,
        Texture2D<float> depthGBuffer)
    {
        positionScreen = params.positionScreen;
        uv             = params.uv;

        float4 sampleAlbedo   = gbufferAlbedo.SampleLevel(samplers[samplerBilinearWrap], uv, 0);
        float4 sampleNormal   = gbufferNormal.SampleLevel(samplers[samplerBilinearWrap], uv, 0);
        float4 sampleMaterial = gbufferMaterial.SampleLevel(samplers[samplerBilinearWrap], uv, 0); // metallic, roughness, emission, occlusion
        float  sampleDepth    = depthGBuffer.SampleLevel(samplers[samplerBilinearWrap], uv, 0);

        albedo    = sampleAlbedo.rgb;
        alpha     = sampleAlbedo.a;
        metallic  = sampleMaterial.r;
        roughness = sampleMaterial.g;
        emission  = sampleMaterial.b;
        occlusion = sampleMaterial.a;

        depth          = sampleDepth;
        normal         = sampleNormal.xyz;

        positionWorld          = ScreenToWorldPosition(uv, depth);
        cameraToPixelDirection = positionWorld - frameData.cameraPosition;
        cameraToPixelDistance  = length(cameraToPixelDirection);
        cameraToPixelDirection = normalize(cameraToPixelDirection);
    }
};

struct Light
{
    uint   flags;
    float3 color;
    float3 position;
    float  intensity;
    float  near;
    float  far;
    float  angle;

    bool IsDirectional() { return (flags & (1 << 0)) != 0; }
    bool IsPoint()       { return (flags & (1 << 1)) != 0; }
    bool IsSpot()        { return (flags & (1 << 2)) != 0; }

    void Build(LightParameters params)
    {
        flags     = params.flags;
        color     = params.color.rgb;
        position  = params.position;
        intensity = params.intensity;
        near      = 0.01f;
        far       = params.range;
        angle     = params.angle;
    }
};

#endif // COMMON_STRUCTS_HLSL