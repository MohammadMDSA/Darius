#include "Common.hlsli"

//#define NO_TANGENT_FRAME

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float3x3 gWorldIT;
};

#ifdef ENABLE_SKINNING
struct Joint
{
    float4x4 PosMatrix;
    float3x3 NrmMatrix; // Inverse-transpose of PosMatrix
};

StructuredBuffer<Joint> Joints : register(t20);
#endif

struct VertexIn
{
    float3 Pos :        POSITION;
    float3 Normal :     NORMAL;
#ifndef NO_TANGENT_FRAME
    float4 Tangent :    TANGENT;
#endif
    float2 UV :         TEXCOORD0;
#ifdef ENABLE_SKINNING
    uint4 JointIndices : BLENDINDICES;
    float4 JointWeights : BLENDWEIGHT;
#endif
};

struct VertexOut
{
#ifndef WORLD_DISPLACEMENT
    float4 Pos :            SV_POSITION;
    float3 WorldPos :       POSITION;
    float3 WorldNormal :    NORMAL;
#else
    float3 LocalPos :       POSITION;
    float3 Normal :         NORMAL;
#endif
#ifndef NO_TANGENT_FRAME
    float4 Tangent :        TANGENT;
#endif
    float2 UV :             TEXCOORD0;
};

[RootSignature(Renderer_RootSig)]
VertexOut main(VertexIn vin)
{
    VertexOut vout;
    
    // Transform to world space
    float4 position = float4(vin.Pos, 1.f);
    float3 normal = vin.Normal;
#ifndef NO_TANGENT_FRAME
    float4 tangent = vin.Tangent;
#endif
    
#ifdef ENABLE_SKINNING
    float4 weights = vin.JointWeights / dot(vin.JointWeights, 1);
    
    float4x4 skinPosMat =
        Joints[vin.JointIndices.x].PosMatrix * weights.x +
        Joints[vin.JointIndices.y].PosMatrix * weights.y +
        Joints[vin.JointIndices.z].PosMatrix * weights.z +
        Joints[vin.JointIndices.w].PosMatrix * weights.w;

    position = mul(skinPosMat, position);

    float3x3 skinNrmMat =
        Joints[vin.JointIndices.x].NrmMatrix * weights.x +
        Joints[vin.JointIndices.y].NrmMatrix * weights.y +
        Joints[vin.JointIndices.z].NrmMatrix * weights.z +
        Joints[vin.JointIndices.w].NrmMatrix * weights.w;

    normal = mul(skinNrmMat, normal).xyz;
    
#ifndef NO_TANGENT_FRAME
    tangent.xyz = mul(skinNrmMat, tangent.xyz).xyz;
#endif
    
#endif
    
#ifndef WORLD_DISPLACEMENT
    vout.WorldPos = mul(gWorld, position).xyz;
    
    float3x3 wit = (float3x3) gWorldIT;
    normal = mul(wit, normal);
    vout.WorldNormal = normal;
    
    // Transform to homogeneous clip space.
    vout.Pos = mul(gViewProj, float4(vout.WorldPos, 1.f));
#else
    vout.LocalPos = position.xyz;
    vout.Normal = normal;
#endif
    
#ifndef NO_TANGENT_FRAME
    vout.Tangent = float4(mul(gWorldIT, tangent.xyz), tangent.w);
#endif
    
    vout.UV = vin.UV;
    
    
    return vout;
}