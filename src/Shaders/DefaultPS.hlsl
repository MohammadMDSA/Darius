#include "Lighting.hlsli"

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
    sample float4 Pos : SV_POSITION;
    sample float3 WorldPos : POSITION;
    sample float3 WorldNormal : NORMAL;
    sample float2 UV : TEXCOORD0;
    
};

Texture2D<float4> texDiffuse                        : register(t0);
Texture2D<float3> texMetallicRoughness              : register(t1);
Texture2D<float3> texOcculusion                     : register(t2);
Texture2D<float3> texEmissive                       : register(t3);
Texture2D<float3> texNormal                         : register(t4);

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
    
    float3 litColor = ComputeLitColor(pin.WorldPos, pin.WorldNormal, toEyeW,
                            diffuseAlbedo, metallicRoughness.x, metallicRoughness.y,
                            emissive, 1, gFresnelR0);
    
    // Common convention to take alpha from diffuse material.
    return float4(litColor, diffuseAlbedo.a);
}
