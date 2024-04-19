#include "../Common.hlsli"

cbuffer cbUuid : register(b0)
{
    uint2 objectUuid;
}

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

[RootSignature(Renderer_RootSig)]
uint2 main(VertexOut pin) : SV_Target
{
    return objectUuid;
}
