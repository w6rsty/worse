// g-buffer
Texture2D gAlbedo   : register(t0);
Texture2D gNormal   : register(t1);
Texture2D gMaterial : register(t2);
Texture2D gDepth    : register(t3);

Texture2D materialTexture[]            : register(t4, space1);
StructuredBuffer<Material> materials[] : register(t5, space2);
StructuredBuffer<Light> lights[]       : register(t6, space3);
