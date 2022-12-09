#include "Common.hlsli"

#define NO_TANGENT_FRAME

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
    float4 Pos :            SV_POSITION;
    float3 WorldPos :       POSITION;
    float3 WorldNormal :    NORMAL;
#ifndef NO_TANGENT_FRAME
    float4 Tangent :        TANGENT;
#endif
    float2 UV :             TEXCOORD0;
};

VertexOut main(VertexIn vin)
{
    VertexOut vout;
    
    // Transform to world space
    float4 position = float4(vin.Pos, 1.f);
    float3 normal = vin.Normal;// * 2 - 1;
#ifndef NO_TANGENT_FRAME
    float4 tangent = vsInput.tangent * 2 - 1
#endif
    
#ifdef ENABLE_SKINNING
    float4x4 skinPosMat =
        Joints[vin.JointIndices.x].PosMatrix * vin.JointWeights.x +
        Joints[vin.JointIndices.y].PosMatrix * vin.JointWeights.y +
        Joints[vin.JointIndices.z].PosMatrix * vin.JointWeights.z +
        Joints[vin.JointIndices.w].PosMatrix * vin.JointWeights.w;

    position = mul(skinPosMat, position);

    float3x3 skinNrmMat =
        Joints[vin.JointIndices.x].NrmMatrix * vin.JointWeights.x +
        Joints[vin.JointIndices.y].NrmMatrix * vin.JointWeights.y +
        Joints[vin.JointIndices.z].NrmMatrix * vin.JointWeights.z +
        Joints[vin.JointIndices.w].NrmMatrix * vin.JointWeights.w;

    normal = mul(skinNrmMat, normal).xyz;
    
#ifndef NO_TANGENT_FRAME
    tangent.xyz = mul(skinNrmMat, tangent.xyz).xyz;
#endif
    
#endif
    
    
    vout.WorldPos = mul(gWorld, position).xyz;
    
    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    //vout.WorldNormal = mul((float3x3) gWorld, normal);
    float3x3 wit = (float3x3) gWorldIT;
    normal = mul(wit, normal);
    vout.WorldNormal = normal;
    // Transform to homogeneous clip space.
    vout.Pos = mul(gViewProj, float4(vout.WorldPos, 1.f));

#ifndef NO_TANGENT_FRAME
    vout.Tangent = float4(mul(gWorldIT, tangent.xyz), tangent.w);
#endif
    
    vout.UV = vin.UV;
    
    
    return vout;
}