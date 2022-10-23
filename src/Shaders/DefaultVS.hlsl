#include "Common.hlsli"

#define NO_TANGENT_FRAME

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float3x3 gWorldIT;
};

#ifdef dd
struct Joint
{
    float4x4 PosMatrix;
    float4x3 NrmMatrix; // Inverse-transpose of PosMatrix
};

StructuredBuffer<Joint> Joints : register(t20);
#endif

struct VertexIn
{
    float3 Pos :        POSITION;
    float3 Normal :     NORMAL;
#ifndef NO_TANGENT_FRAME
    float4 Tangent : TANGENT;
#endif
    float2 UV :         TEXCOORD0;
#ifdef ENABLE_SKINNING
    uint4 JointIndices : BLENDINDICES;
    float4 JointWeights : BLENDWEIGHT;
#endif
};

struct VertexOut
{
    float4 Pos :            SV_POSITION;
    float3 WorldPos :       POSITION;
    float3 WorldNormal :    NORMAL;
#ifndef NO_TANGENT_FRAME
    float4 tangent : TANGENT;
#endif
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
    
#ifdef dd
    // I don't like this hack.  The weights should be normalized already, but something is fishy.
    float4 weights = vsInput.jointWeights / dot(vsInput.jointWeights, 1);

    float4x4 skinPosMat =
        Joints[vsInput.jointIndices.x].PosMatrix * weights.x +
        Joints[vsInput.jointIndices.y].PosMatrix * weights.y +
        Joints[vsInput.jointIndices.z].PosMatrix * weights.z +
        Joints[vsInput.jointIndices.w].PosMatrix * weights.w;

    position = mul(skinPosMat, position);

    float4x3 skinNrmMat =
        Joints[vsInput.jointIndices.x].NrmMatrix * weights.x +
        Joints[vsInput.jointIndices.y].NrmMatrix * weights.y +
        Joints[vsInput.jointIndices.z].NrmMatrix * weights.z +
        Joints[vsInput.jointIndices.w].NrmMatrix * weights.w;

    normal = mul(skinNrmMat, normal).xyz;

#endif
    
    return vout;
}