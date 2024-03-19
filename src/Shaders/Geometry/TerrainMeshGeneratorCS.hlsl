#include "../CommonRS.hlsli"
#include "../Utils/ShaderUtility.hlsli"

#define HLSL

#include "VertexType.h"

cbuffer CB0                                 : register(b1)
{
    float           HeightFactor;
    float2          DisplacementTexInvSize;
}

Texture2D<float>                HeightMap   : register(t0);

RWStructuredBuffer<VertexPositionNormalTangentTexture> Vertices : register(u0);

float getHeight(float2 uv)
{
    return HeightMap.SampleLevel(LinearSampler, uv, 0.f).x;
}

[RootSignature(Common_RootSig)]
[numthreads(8, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    VertexPositionNormalTangentTexture vert = Vertices.Load(index);
    
    float height = HeightFactor * getHeight(vert.Tex);
    vert.Position.y = height;
    
    // Calc vertex normal
    float3 vertNormal;
    {
        float3 off = float3(1.f * DisplacementTexInvSize.x, 0.f, 1.f * DisplacementTexInvSize.y);
        float hL = getHeight(vert.Tex - off.xy);
        float hR = getHeight(vert.Tex + off.xy);
        float hD = getHeight(vert.Tex - off.yz);
        float hU = getHeight(vert.Tex + off.yz);

        // Deduce terrain normal
        float3 normal; // = normalize(ResolveParam(Normal));
        normal.x = (hL - hR) * HeightFactor;
        normal.z = (hD - hU) * HeightFactor;
        normal.y = 2.0;
        normal = normalize(normal);
        vertNormal = normalize(normal);
    }
    
    float3 normalWorld = BumpMapNormalToWorldSpaceNormal(vertNormal, vert.Normal, vert.Tangent.xyz);
    vert.Normal = normalWorld;

    Vertices[index] = vert;
}