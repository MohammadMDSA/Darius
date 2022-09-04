#include "Common.hlsli"
#include "Lighting.hlsli"

#define BitMasked(value, bitIdx) value & (1 << bitIdx)

cbuffer cbMaterial : register(b0)
{
    float4  gDiffuseAlbedo;
    float3  gFresnelR0;
    float   gRoughness;
    int     gTexStats;
};

cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float3 gCameraPosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;
    
};

struct VertexOut
{
    sample float4 Pos           : SV_POSITION;
    sample float3 WorldPos      : POSITION;
    sample float3 WorldNormal   : NORMAL;
    sample float2 UV            : TEXCOORD0;
    
};

Texture2D<float4>       texDiffuse      : register(t0);
Texture2D<float>        texRoughness    : register(t1);
Texture2D<float3>       texOcculusion   : register(t2);
Texture2D<float3>       texEmissive     : register(t3);
Texture2D<float3>       texNormal       : register(t4);

float4 main(VertexOut pin) : SV_Target
{
# define SAMPLE_TEX(texName) texName.Sample(defaultSampler, pin.UV)
    
    // Interpolating normal can unnormalize it, so renormalize it.
    pin.WorldNormal = normalize(pin.WorldNormal);

    // Vector from point being lit to eye. 
    float3 toEyeW = normalize(gCameraPosW - pin.WorldPos);

    float4 diffuseAlbedo;
    
    if(BitMasked(gTexStats, 0))
        diffuseAlbedo = SAMPLE_TEX(texDiffuse);
    else
        diffuseAlbedo = gDiffuseAlbedo;
    
    // Direct Lightin
    float4 ambient = gAmbientLight * diffuseAlbedo;

    float roughness;
    if (BitMasked(gTexStats, 1))
        roughness = SAMPLE_TEX(texRoughness);
    else
        roughness = gRoughness;
    
    const float shininess = 1.0f - roughness;
    Material mat = { diffuseAlbedo, gFresnelR0, shininess, gTexStats };
    float3 shadowFactor = 1.0f;

    float4 directLight = ComputeLighting(mat, pin.WorldPos,
        pin.WorldNormal, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

    // Common convention to take alpha from diffuse material.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}
