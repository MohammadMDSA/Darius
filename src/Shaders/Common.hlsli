#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

// Common (static) samplers
SamplerState defaultSampler : register(s10);
SamplerComparisonState shadowSampler : register(s11);
SamplerState cubeMapSampler : register(s12);

#endif