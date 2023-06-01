#include "../../Common.hlsli"

struct VertexIn
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
#ifndef NO_TANGENT_FRAME
    float4 Tangent : TANGENT;
#endif
    float2 UV : TEXCOORD0;
};

struct VertexOut
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
#ifndef NO_TANGENT_FRAME
    float4 Tangent : TANGENT;
#endif
    float2 UV : TEXCOORD0;
};

struct HullOut
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
#ifndef NO_TANGENT_FRAME
    float4 Tangent : TANGENT;
#endif
    float2 UV : TEXCOORD0;
};

struct PatchTess
{
    float EdgeTess[4] : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};

struct DomainOut
{
    float4 Pos : SV_POSITION;
    float3 WorldPos : POSITION;
    float3 WorldNormal : NORMAL;
#ifndef NO_TANGENT_FRAME
    float4 Tangent : TANGENT;
#endif
    float2 UV : TEXCOORD0;
};

