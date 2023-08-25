#include "../CommonRS.hlsli"

#define HLSL

#include "VertexType.h"

cbuffer CB0
{
    float           HeightFactor;
}

Texture2D<float>                HeightMap   : register(t0);

RWStructuredBuffer<RTVertexPositionNormalTangentTexture> Vertices : register(u0);

[RootSignature(Common_RootSig)]
[numthreads(8, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    RTVertexPositionNormalTangentTexture vert = Vertices.Load(index);
    
    float height = HeightFactor * HeightMap.SampleLevel(LinearSampler, vert.Tex, 0).x;

    vert.Position.y = height;
    Vertices[index] = vert;
}