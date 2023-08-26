#include "../CommonRS.hlsli"
#include "../Utils/ShaderUtility.hlsli"

#define HLSL

#include "VertexType.h"

cbuffer CB0
{
    float           HeightFactor;
}

Texture2D<float>                HeightMap   : register(t0);
Texture2D<float3>               NormalMap   : register(t1);

RWStructuredBuffer<RTVertexPositionNormalTangentTexture> Vertices : register(u0);

[RootSignature(Common_RootSig)]
[numthreads(8, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    RTVertexPositionNormalTangentTexture vert = Vertices.Load(index);
    
    float height = HeightFactor * HeightMap.SampleLevel(LinearSampler, vert.Tex, 0).x;
    vert.Position.y = height;
    
    float3 normalSample = NormalMap.SampleLevel(LinearSampler, vert.Tex, 0).xyz;
    float3 normalWorld = BumpMapNormalToWorldSpaceNormal(normalize(normalSample * 2.f - 1.f), vert.Normal, vert.Tangent.xyz);
    vert.Normal = normalWorld;

    Vertices[index] = vert;
}