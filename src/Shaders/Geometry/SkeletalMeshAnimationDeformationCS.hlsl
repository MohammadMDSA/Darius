#define HLSL

#include "../CommonRS.hlsli"
#include "VertexType.h"
#include "Joint.hlsli"

RWStructuredBuffer<VertexPositionNormalTangentTexture>          OutVertices     : register(u0);

StructuredBuffer<VertexPositionNormalTangentTextureSkinned>     InVertices      : register(t0);
StructuredBuffer<Joint>                                         Joints          : register(t1);

[RootSignature(Common_RootSig)]
[numthreads(256, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    VertexPositionNormalTangentTextureSkinned vin = InVertices[index];
    VertexPositionNormalTangentTexture vout;
    
    float4 position = float4(vin.Position, 1.f);
    float3 normal = vin.Normal;
    float4 tangent = vin.Tangent;
    
    float4 weights = vin.BlendWeights / dot(vin.BlendWeights, 1);
    
    float4x4 skinPosMat =
        Joints[vin.BlendIndices.x].PosMatrix * weights.x +
        Joints[vin.BlendIndices.y].PosMatrix * weights.y +
        Joints[vin.BlendIndices.z].PosMatrix * weights.z +
        Joints[vin.BlendIndices.w].PosMatrix * weights.w;

    vout.Position = mul(skinPosMat, position).xyz;

    float3x3 skinNrmMat =
        Joints[vin.BlendIndices.x].NrmMatrix * weights.x +
        Joints[vin.BlendIndices.y].NrmMatrix * weights.y +
        Joints[vin.BlendIndices.z].NrmMatrix * weights.z +
        Joints[vin.BlendIndices.w].NrmMatrix * weights.w;
   
    vout.Normal = mul(skinNrmMat, normal).xyz;
    
    vout.Tangent = float4(mul(skinNrmMat, tangent.xyz).xyz, vin.Tangent.w);
    vout.Tex = vin.Tex;
    
    OutVertices[index] = vout;
}
