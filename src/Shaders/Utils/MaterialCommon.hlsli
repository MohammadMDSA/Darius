#ifndef __MATERIAL_COMMON_HLSLI__
#define __MATERIAL_COMMON_HLSLI__

namespace MaterialTextureType
{
    enum TextureType : uint16_t
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


#endif