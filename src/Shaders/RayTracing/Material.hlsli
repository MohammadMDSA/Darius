#ifndef __RT_MATERIAL_HLSLI__
#define __RT_MATERIAL_HLSLI__

namespace MaterialTextureType
{
    enum TextureType
	{
		Diffuse = 0,
		Metallic,
		Roughness,
		AmbientOcclusion,
		Emissive,
		Normal,
		WorldDisplacement,

		NumTextures
	};
}
    
namespace MaterialType {
    enum Type {
        Default,
        Matte,      // Lambertian scattering
        Mirror,     // Specular reflector that isn't modified by the Fresnel equations.
    };
}
        
struct PrimitiveMaterialBuffer
{
    XMFLOAT3                Albedo;
    XMFLOAT3                Specular;
    XMFLOAT3                Metallic;
    XMFLOAT3                Transmissivity;
    XMFLOAT3                Emission;
    XMFLOAT3                Opacity;
    XMFLOAT3                Eta;
    float                   Rroughness;
    MaterialType::Type      Type;
};

cbuffer                             l_MaterialConstants         : register(b1, space2)
{
    float4                          l_DiffuseAlbedo;
    float3                          l_FresnelR0;
    float3                          l_Emissive;
    float                           l_Metallic;
    float                           l_Roughness;
    float                           l_DisplacementAmount;
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