#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

#define Renderer_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
    "CBV(b0, visibility = SHADER_VISIBILITY_VERTEX), " \
    "CBV(b0, visibility = SHADER_VISIBILITY_HULL), " \
    "CBV(b0, visibility = SHADER_VISIBILITY_DOMAIN), " \
    "CBV(b2, visibility = SHADER_VISIBILITY_DOMAIN), " \
    "CBV(b0, visibility = SHADER_VISIBILITY_PIXEL), " \
    "DescriptorTable(SRV(t0, numDescriptors = 10), visibility = SHADER_VISIBILITY_DOMAIN)," \
    "DescriptorTable(SRV(t0, numDescriptors = 10), visibility = SHADER_VISIBILITY_GEOMETRY)," \
    "DescriptorTable(SRV(t0, numDescriptors = 10), visibility = SHADER_VISIBILITY_PIXEL)," \
    "DescriptorTable(Sampler(s0, numDescriptors = 10), visibility = SHADER_VISIBILITY_PIXEL)," \
    "DescriptorTable(SRV(t10, numDescriptors = 10), visibility = SHADER_VISIBILITY_PIXEL)," \
    "CBV(b1), " \
    "CBV(b10, visibility = SHADER_VISIBILITY_PIXEL), " \
    "SRV(t20, visibility = SHADER_VISIBILITY_VERTEX), " \
    "StaticSampler(s10, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_PIXEL)," \
    "StaticSampler(s11, visibility = SHADER_VISIBILITY_PIXEL," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "comparisonFunc = COMPARISON_GREATER_EQUAL," \
        "filter = FILTER_MIN_MAG_LINEAR_MIP_POINT)," \
    "StaticSampler(s12, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_PIXEL)," \
    "StaticSampler(s13, maxAnisotropy = 16)," \
    "StaticSampler(s14, maxAnisotropy = 16)"

// Common (static) samplers
SamplerState defaultSampler             : register(s10);
SamplerComparisonState shadowSampler    : register(s11);
SamplerState cubeMapSampler             : register(s12);
SamplerState linearWrap                 : register(s13);
SamplerState linearClamp                : register(s14);

cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float4x4 gInvViewProjEyeCenter;
    float4 gFrustumPlanes[6];
    float3 gCameraPosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;
    float IBLRange;
    float IBLBias;
    
};

#endif