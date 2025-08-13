#include "Common.hlsl"

StructuredBuffer<Material> materials : register(t0, space1);

struct VertexOutput
{
    float4 position  : SV_Position;
    float2 uv        : TEXCOORD0;
    float3 worldPos  : POSITION;
    float3 normal    : NORMAL;
    float3 tangent   : TANGENT;
    float3 bitangent : TANGENT1;
};

VertexOutput main_vs(VertexPosUvNrmTan input)
{
    VertexOutput output;

    float4x4 mvp = mul(frameData.viewProjection, pushData.transform);
    output.position = mul(mvp, float4(input.position, 1.0));
    
    // Transform to world space
    output.worldPos = mul(pushData.transform, float4(input.position, 1.0)).xyz;
    output.uv = input.uv;
    
    // Transform normals to world space
    float3x3 normalMatrix = (float3x3)pushData.transform;
    output.normal = normalize(mul(input.normal, normalMatrix));
    output.tangent = normalize(mul(input.tangent.xyz, normalMatrix)) * input.tangent.w;
    output.bitangent = normalize(cross(output.normal, output.tangent));

    // hack to avoid z-fighting
    output.position.z += 1e-6;

    return output;
}

struct PixelOutput
{
    float4 color : SV_Target0;
    float4 normal : SV_Target1;
    float4 albedo : SV_Target2;
};

// Constants
static const float PI = 3.14159265359;

// Fresnel-Schlick approximation
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

// Geometry function
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

PixelOutput main_ps(VertexOutput input)
{
    PixelOutput output;

    Material material = materials[getMaterialId()];
    
    // Sample textures
    float3 baseColor = material.baseColor.rgb;
    float metallic = material.metallic;
    float roughness = material.roughness;
    float ao = material.ambientOcclusion;
    
    float3 textureBaseColor = materialTextures[material.baseColorTextureIndex].Sample(samplers[samplerBilinearWrap], input.uv).rgb;
    baseColor *= textureBaseColor;

    float2 metallicRoughness = materialTextures[material.metallicRoughnessTextureIndex].Sample(samplers[samplerBilinearWrap], input.uv).rg;
    metallic *= metallicRoughness.x;
    roughness *= metallicRoughness.y;

    ao = materialTextures[material.ambientOcclusionTextureIndex].Sample(samplers[samplerBilinearWrap], input.uv).r;

    float3 emissive = materialTextures[material.emissiveTextureIndex].Sample(samplers[samplerBilinearWrap], input.uv).rgb;

    // Normal mapping
    float3 N = normalize(input.normal);
    float3 normalMap = materialTextures[material.normalTextureIndex].Sample(samplers[samplerBilinearWrap], input.uv).rgb;
    normalMap = normalMap * 2.0 - 1.0; // Convert from [0,1] to [-1,1]
    
    // // Construct TBN matrix for normal mapping
    float3x3 TBN = float3x3(
        normalize(input.tangent),
        normalize(input.bitangent),
        normalize(input.normal)
    );
    
    // Transform normal from tangent space to world space
    N = normalize(mul(normalMap, TBN));
    
    float3 lightDirection = frameData.cameraForward;
    float3 const lightColor = float3(1.0, 1.0, 1.0);
    float const lightIntensity = 1.0;
    
    float3 V = normalize(frameData.cameraPosition - input.worldPos); // View direction
    float3 L = -lightDirection; // Light direction
    float3 H = normalize(V + L); // Half vector
    
    // No attenuation for directional light
    float attenuation = 1.0;
    
    // Cook-Torrance BRDF
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), baseColor, metallic);
    
    // Calculate Cook-Torrance BRDF components
    float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // Prevent divide by zero
    float3 specular = numerator / denominator;
    
    // Energy conservation
    float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - metallic;
    
    float NdotL = max(dot(N, L), 0.0);
    
    // Diffuse component
    float3 diffuse = kD * baseColor / PI;
    
    // Combine lighting
    float3 radiance = lightColor * lightIntensity * attenuation;
    float3 Lo = (diffuse + specular) * radiance * NdotL;
    
    // Ambient lighting
    float3 ambient = float3(0.03, 0.03, 0.03) * baseColor * ao;

    float3 color = emissive + Lo + ambient;

    output.color = float4(color, 1.0);
    output.normal = float4(N, 1.0);

    return output;
}