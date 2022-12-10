#include "Lighting.hlsli"

#define NO_TANGENT_FRAME

#define BitMasked(value, bitIdx) value & (1 << bitIdx)

cbuffer cbMaterial : register(b0)
{
    float4 gDiffuseAlbedo;
    float3 gFresnelR0;
    float3 gEmissive;
    float2 gMetallicRoughness;
    int gTexStats;
};

struct VertexOut
{
    float4 Pos : SV_POSITION;
    float3 WorldPos : POSITION;
    float3 WorldNormal : NORMAL;
#ifndef NO_TANGENT_FRAME
    float4 Tangent : TANGENT;
#endif
    float2 UV : TEXCOORD0;
    
};

Texture2D<float4> texDiffuse                        : register(t0);
Texture2D<float3> texMetallicRoughness              : register(t1);
Texture2D<float3> texOcculusion                     : register(t2);
Texture2D<float3> texEmissive                       : register(t3);
Texture2D<float3> texNormal                         : register(t4);

float3 ComputeNormal(VertexOut pin)
{
    float3 normal = normalize(pin.WorldNormal);

#ifdef NO_TANGENT_FRAME
    return normal;
#else
    // Construct tangent frame
    float3 tangent = normalize(pin.Tangent.xyz);
    float3 bitangent = normalize(cross(normal, tangent)) * pin.Tangent.w;
    float3x3 tangentFrame = float3x3(tangent, bitangent, normal);

    // Read normal map and convert to SNORM (TODO:  convert all normal maps to R8G8B8A8_SNORM?)
    normal = texNormal.Sample(defaultSampler, pin.UV) * 2.0 - 1.0;

    // glTF spec says to normalize N before and after scaling, but that's excessive
    //normal = normalize(normal * float3(normalTextureScale, normalTextureScale, 1));

    // Multiply by transpose (reverse order)
    return mul(normal, tangentFrame);
#endif
}

[RootSignature(Renderer_RootSig)]
float4 main(VertexOut pin) : SV_Target
{
#define SAMPLE_TEX(texName) texName.Sample(defaultSampler, pin.UV)
    
    // Interpolating normal can unnormalize it, so renormalize it.
    pin.WorldNormal = normalize(pin.WorldNormal);

    // Vector from point being lit to eye. 
    float3 toEyeW = normalize(gCameraPosW - pin.WorldPos);

    float4 diffuseAlbedo;
    
    // Diffuse Albedo
    if (BitMasked(gTexStats, 0))
        diffuseAlbedo = SAMPLE_TEX(texDiffuse);
    else
        diffuseAlbedo = gDiffuseAlbedo;
    
    // Direct Lightin
    float4 ambient = gAmbientLight * diffuseAlbedo;

    // Roughness
    float2 metallicRoughness;
    if (BitMasked(gTexStats, 1))
        metallicRoughness = SAMPLE_TEX(texMetallicRoughness).xy;
    else
        metallicRoughness = gMetallicRoughness;
    
    // Emissive 
    float3 emissive;
    if (BitMasked(gTexStats, 3))
        emissive = SAMPLE_TEX(texEmissive);
    else
        emissive = gEmissive;
    
    float3 normal;
    if (BitMasked(gTexStats, 4))
        normal = ComputeNormal(pin);
    else
        normal = pin.WorldNormal;
    
    float3 litColor = ComputeLitColor(pin.WorldPos, normal, toEyeW,
                            diffuseAlbedo, metallicRoughness.x, metallicRoughness.y,
                            emissive, 1, gFresnelR0);
    
    // Common convention to take alpha from diffuse material.
    return float4(litColor, diffuseAlbedo.a);
}
