#ifndef __RT_MATERIAL_BINDINGS_HLSLI__
#define __RT_MATERIAL_BINDINGS_HLSLI__


#include "RayTracingMaterial.hlsli"

cbuffer                             l_MaterialConstants         : register(b1, space2)
{
    float4                          l_DiffuseAlbedo;
    float4                          l_FresnelR0;
    float3                          l_Emissive;
    float                           l_Metallic;
    float                           l_Roughness;
    float                           l_DisplacementAmount;
    float                           l_Opacity;
    uint                            l_TexStats;
};
     
Texture2D<float4>                   l_TexDiffuse                : register(t0, space2);
Texture2D<float>                    l_TexMetallic               : register(t1, space2);
Texture2D<float>                    l_TexRoughness              : register(t2, space2);
Texture2D<float>                    l_TexAmbientOcclusion       : register(t3, space2);
Texture2D<float3>                   l_TexEmissive               : register(t4, space2);
Texture2D<float3>                   l_TexNormal                 : register(t5, space2);
Texture2D<float>                    l_TexWorldDisplacement      : register(t6, space2);

SamplerState                        l_DiffuseSampler            : register(s0, space2);
SamplerState                        l_MetallicSampler           : register(s1, space2);
SamplerState                        l_RoughnessSampler          : register(s2, space2);
SamplerState                        l_OcclusionSampler          : register(s3, space2);
SamplerState                        l_EmissiveSampler           : register(s4, space2);
SamplerState                        l_NormalSampler             : register(s5, space2);
SamplerState                        l_WorldDisplacementSampler  : register(s6, space2);
	
#endif