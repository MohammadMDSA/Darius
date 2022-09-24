#include "Common.hlsli"

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float3x3 gWorldIT;
};

struct VertexIn
{
    float3 Pos :        POSITION;
    float3 Normal :     NORMAL;
    float2 UV :         TEXCOORD0;
};

struct VertexOut
{
    float4 Pos :            SV_POSITION;
    float3 WorldPos :       POSITION;
    float3 WorldNormal :    NORMAL;
    float2 UV :             TEXCOORD0;
};

VertexOut main(VertexIn vin)
{
    VertexOut vout;
    
    // Transform to world space
    float4 posW = mul(gWorld, float4(vin.Pos, 1.f));
    vout.WorldPos = posW.xyz;
    
    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.WorldNormal = mul((float3x3)gWorld, vin.Normal);

    // Transform to homogeneous clip space.
    vout.Pos = mul(gViewProj, posW);

    vout.UV = vin.UV;
    
    return vout;
}