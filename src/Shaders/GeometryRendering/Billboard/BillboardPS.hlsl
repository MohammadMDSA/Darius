#include "../../Common.hlsli"

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

[RootSignature(Renderer_RootSig)]
MRT main(GSOutput gout) : SV_Target
{
    MRT result;
    
    result.Color = float4(1.f, 0.f, 0.f, 1.f);
    result.Normal = (gout.WorldNormal, 1.f);
    return result;
}