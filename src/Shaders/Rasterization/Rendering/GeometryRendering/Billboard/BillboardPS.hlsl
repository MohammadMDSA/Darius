#include "../../Lighting.hlsli"

#include "../../../../Utils/MaterialCommon.hlsli"

#define BitMasked(value, bitIdx) value & (1u << bitIdx)


cbuffer cbMaterial : register(b0)
{
    float4 gDiffuseAlbedo;
    float3 gFresnelR0;
    float3 gEmissive;
    float  gMetallic;
    float  gRoughness;
    float  gDisplacementAmount;
    float  gOpacity;
    float  gSpecular;
    uint   gTexStats;
};

Texture2D<float4> texDiffuse : register(t0);
Texture2D<float> texMetallic : register(t1);
Texture2D<float> texRoughness : register(t2);
Texture2D<float> texAmbientOcclusion : register(t3);
Texture2D<float3> texEmissive : register(t4);
Texture2D<float3> texNormal : register(t5);

SamplerState DiffuseSampler : register(s0);
SamplerState MetallicSampler : register(s1);
SamplerState RoughnessSampler : register(s2);
SamplerState OcclusionSampler : register(s3);
SamplerState EmissiveSampler : register(s4);
SamplerState NormalSampler : register(s5);

struct MRT
{
    float4 Color : SV_Target0;
    float4 Normal : SV_Target1;
};

struct GSOutput
{
    float4 Pos : SV_POSITION;
    float3 WorldPos : POSITION;
    float3 WorldNormal : NORMAL;
#ifndef NO_TANGENT_FRAME
    float4 Tangent : TANGENT;
#endif
    float2 UV : TEXCOORD0;
    
};

float3 ComputeNormal(GSOutput gout)
{
    float3 normal = normalize(gout.WorldNormal);

#ifdef NO_TANGENT_FRAME
    return normal;
#else
    
    // Construct tangent frame
    float3 tangent = normalize(gout.Tangent.xyz);
    float3 bitangent = normalize(cross(normal, tangent)) * gout.Tangent.w;
    float3x3 tangentFrame = float3x3(tangent, bitangent, normal);

    // Read normal map and convert to SNORM (TODO:  convert all normal maps to R8G8B8A8_SNORM?)
    normal = texNormal.Sample(NormalSampler, gout.UV) * 2.f - 1.f;
    
    // glTF spec says to normalize N before and after scaling, but that's excessive
    //normal = normalize(normal * float3(normalTextureScale, normalTextureScale, 1));

    // Multiply by transpose (reverse order)
    return normalize(mul(normal, tangentFrame));
#endif
}

[RootSignature(Renderer_RootSig)]
MRT main(GSOutput gout) : SV_Target
{
#define SAMPLE_TEX(texName, sampler) texName.Sample(sampler, gout.UV)
    
    MRT mrt;
    
    // Interpolating normal can unnormalize it, so renormalize it.
    gout.WorldNormal = normalize(gout.WorldNormal);

    // Vector from point being lit to eye. 
    float3 toEyeW = normalize(gCameraPosW - gout.WorldPos);

    float4 diffuseAlbedo;
    
    // Diffuse Albedo
    if (BitMasked(gTexStats, MaterialTextureType::Diffuse))
        diffuseAlbedo = SAMPLE_TEX(texDiffuse, DiffuseSampler);
    else
        diffuseAlbedo = gDiffuseAlbedo;
    
    // Metallic
    float metallic;
    if (BitMasked(gTexStats, MaterialTextureType::Metallic))
        metallic = SAMPLE_TEX(texMetallic, MetallicSampler).x;
    else
        metallic = gMetallic;
    
    // Roughness
    float roughness;
    if (BitMasked(gTexStats, MaterialTextureType::Roughness))
        roughness = SAMPLE_TEX(texRoughness, RoughnessSampler);
    else
        roughness = gRoughness;
    
    // Emissive 
    float3 emissive;
    if (BitMasked(gTexStats, MaterialTextureType::Emissive))
        emissive = SAMPLE_TEX(texEmissive, EmissiveSampler);
    else
        emissive = gEmissive;
    
    float3 normal;
    if (BitMasked(gTexStats, MaterialTextureType::Normal))
        normal = ComputeNormal(gout);
    else
        normal = gout.WorldNormal;
    
    float ao;
    if (BitMasked(gTexStats, MaterialTextureType::AmbientOcclusion))
        ao = SAMPLE_TEX(texAmbientOcclusion, OcclusionSampler);
    else
        ao = 1;
    
    float3 litColor = ComputeLitColor(gout.WorldPos, uint2(gout.Pos.xy), normal,
                            toEyeW, diffuseAlbedo, metallic, roughness,
                            emissive, ao, gSpecular, gFresnelR0);
    

    float opacity = gOpacity;
    if(opacity == 0.f)
        opacity = diffuseAlbedo.a;
            
    // Common convention to take alpha from diffuse material.
    mrt.Color = float4(litColor, opacity);
    mrt.Normal = float4(gout.WorldNormal, 1.f);
    
    return mrt;
}