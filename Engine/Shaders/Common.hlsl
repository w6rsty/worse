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