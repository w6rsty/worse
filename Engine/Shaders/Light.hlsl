#include "Common.hlsl"
#include "ShadowMapping.hlsl"
#include "Utils.hlsl"

Texture2D<float4> gbufferAlbedo   : register(t0, space1);
Texture2D<float4> gbufferNormal   : register(t1, space1);
Texture2D<float4> gbufferMaterial : register(t2, space1);
Texture2D<float> depthGBuffer     : register(t3, space1);
Texture2D<float> depthLight       : register(t4, space1);
RWTexture2D<float4> output        : register(u0, space1);

float3 CalculatePBRLighting(Surface surface, LightParameters light, Texture2D<float> shadowMap)
{
    float3 N = normalize(surface.normal);
    float3 V = normalize(frameData.cameraPosition - surface.positionWorld);
    float3 lightDir;
    float attenuation = 1.0;
    
    // 计算光线方向和衰减
    if (light.flags & (1 << 0)) // 方向光
    {
        lightDir = normalize(-light.direction);
    }
    else // 点光源/聚光灯
    {
        float3 lightToPixel = surface.positionWorld - light.position;
        float distance      = length(lightToPixel);
        lightDir            = -normalize(lightToPixel);
        
        // 距离衰减
        attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
        attenuation = min(attenuation, 1.0 / max(distance - light.range, 0.001));
        
        // 聚光灯衰减
        if (light.flags & (1 << 2)) // 聚光灯
        {
            float3 spotDir  = normalize(light.direction);
            float theta     = dot(-lightDir, spotDir);
            float epsilon   = cos(light.angle * 0.5) - cos(light.angle);
            float intensity = clamp((theta - cos(light.angle)) / epsilon, 0.0, 1.0);
            attenuation *= intensity;
        }
    }
    
    float3 H = normalize(V + lightDir);
    
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, lightDir), 0.0);
    float HdotV = max(dot(H, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), surface.albedo, surface.metallic);
    
    // Cook-Torrance BRDF
    float3 F  = FresnelSchlick(HdotV, F0);
    float NDF = DistributionGGX(N, H, surface.roughness);
    float G   = GeometrySmith(N, V, lightDir, surface.roughness);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.001;
    float3 specular = numerator / denominator;
    
    float3 kS  = F;
    float3 kD  = float3(1.0, 1.0, 1.0) - kS;
    kD        *= 1.0 - surface.metallic;
    
    float3 diffuse = kD * surface.albedo / PI;
    
    // float shadow = ComputeShadow(shadowMap, surface, light);
    
    float3 radiance = light.color.rgb * light.intensity * attenuation;
    float3 color = (diffuse + specular) * radiance * NdotL;
    
    return color;
}

float3 CalculateAmbientLighting(Surface surface, float3 ambientColor)
{
    float3 ambient = ambientColor * surface.albedo * surface.occlusion;
    return ambient;
}

[numthreads(THREAD_GROUP_COUNT_X, THREAD_GROUP_COUNT_Y, 1)]
void main_cs(uint3 threadID : SV_DispatchThreadID)
{
    float2 resolution;
    output.GetDimensions(resolution.x, resolution.y);

    if (any(threadID.xy >= resolution))
    {
        return;
    }

    float2 uv = (threadID.xy + 0.5) / resolution;
    SurfaceParameters surfaceParams;
    surfaceParams.positionScreen = threadID.xy;
    surfaceParams.uv             = uv;

    Surface surface;
    surface.Build(surfaceParams, gbufferAlbedo, gbufferNormal, gbufferMaterial, depthGBuffer);

    if (surface.depth >= 1.0)
    {
        output[threadID.xy] = float4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    // Debug hack
    LightParameters lights[3];
    // directional
    lights[0].color = float4(1.0, 0.95, 0.8, 1.0);
    lights[0].position = float3(0.0, 0.0, 0.0);
    lights[0].intensity = 1.0;
    lights[0].direction = normalize(float3(-0.3, -1.0, -0.5));
    lights[0].range = 1000.0;
    lights[0].angle = 0.0;
    lights[0].flags = (1 << 0);
    lights[0].padding = float2(0.0, 0.0);
    
    // spot1
    lights[1].color = float4(1.0, 0.6, 0.3, 1.0);
    lights[1].position = float3(0.0, 2.0, 3.0);
    lights[1].intensity = 8.0;
    lights[1].direction = float3(0.0, 0.0, 0.0);
    lights[1].range = 100.0;
    lights[1].angle = 0.0;
    lights[1].flags = (1 << 1);
    lights[1].padding = float2(0.0, 0.0);

    // spot2
    lights[2].color = float4(0.3, 0.6, 1.0, 1.0);
    lights[2].position = float3(-2.0, 2.0, 0.0);
    lights[2].intensity = 5.0;
    lights[2].direction = float3(0.0, 0.0, 0.0);
    lights[2].range = 100.0;
    lights[2].angle = 0.0;
    lights[2].flags = (1 << 2);
    lights[2].padding = float2(0.0, 0.0);

    float3 finalColor = float3(0.0, 0.0, 0.0);
    finalColor += CalculateAmbientLighting(surface, float3(0.01, 0.01, 0.01));
    finalColor += surface.albedo * surface.emission;
    
    for (uint i = 0; i < 3; ++i)
    {
        finalColor += CalculatePBRLighting(surface, lights[i], depthLight);
    }

    output[threadID.xy] = float4(finalColor, surface.alpha);
}