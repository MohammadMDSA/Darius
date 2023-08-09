#ifndef __FRESNEL_HLSLI__
#define __FRESNEL_HLSLI__

inline float FresnelSchlick(float f0, float f90, float cosine)
{
	return lerp(f0, f90, pow(max(1.f - cosine, 0), 5.f));
}

inline float3 FresnelSchlick(float3 f0, float3 f90, float cosine)
{
	return lerp(f0, f90, pow(max(1.f - cosine, 0), 5.f));
}

inline float3 FresnelGeneralizedSchlick(float f0, float3 f90, float exp, float cosine)
{
	return lerp(f0, f90, pow(max(1 - cosine, 0), exp));
}

inline float3 FresnelSchlickNormalRange(float3 f0, float cosine)
{
	return FresnelSchlick(f0, 1, cosine);
}

inline float FresnelSchlickNormalRange(float f0, float cosine)
{
	return FresnelSchlick(f0, 1, cosine);
}

#endif