#include "Lighting.hlsli"

//#define NO_TANGENT_FRAME

#define BitMasked(value, bitIdx) value & (1 << bitIdx)

cbuffer cbMaterial : register(b0)
{
    float4 gDiffuseAlbedo;
    float3 gFresnelR0;
    float3 gEmissive;
    float1 gMetallic;
    float1 gRoughness;
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
Texture2D<float3> texMetallic                       : register(t1);
Texture2D<float1> texRoughness                      : register(t2);
Texture2D<float1> texOcculusion                     : register(t3);
Texture2D<float3> texEmissive                       : register(t4);
Texture2D<float3> texNormal                         : register(t5);

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
    normal = texNormal.Sample(defaultSampler, pin.UV);

    // glTF spec says to normalize N before and after scaling, but that's excessive
    //normal = normalize(normal * float3(normalTextureScale, normalTextureScale, 1));

    // Multiply by transpose (reverse order)
    return mul(normal, tangentFrame);
#endif
}

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
    
    // Metallic
    float metallic;
    if (BitMasked(gTexStats, 1))
        metallic = SAMPLE_TEX(texMetallic).x;
    else
        metallic = gMetallic;
    
    // Roughness
    float roughness;
    if (BitMasked(gTexStats, 2))
        roughness = SAMPLE_TEX(texRoughness).x;
    else
        roughness = gRoughness;
    
    // Emissive 
    float3 emissive;
    if (BitMasked(gTexStats, 4))
        emissive = SAMPLE_TEX(texEmissive);
    else
        emissive = gEmissive;
    
    float3 normal;
    if (BitMasked(gTexStats, 5))
        normal = ComputeNormal(pin);
    else
        normal = pin.WorldNormal;
    
    float3 litColor = ComputeLitColor(pin.WorldPos, normal, toEyeW,
                            diffuseAlbedo, metallic, roughness,
                            emissive, 1, gFresnelR0);
    
    // Common convention to take alpha from diffuse material.
    return float4(litColor, diffuseAlbedo.a);
}
