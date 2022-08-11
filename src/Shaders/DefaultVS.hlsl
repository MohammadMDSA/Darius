#include "Common.hlsli"
#include "Lighting.hlsli"

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
};

cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float3 gCameraPosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;
    
    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light gLights[MAX_LIGHTS];
};

struct VertexIn
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
};

struct VertexOut
{
    float4 Pos : SV_POSITION;
    float3 WorldPos : POSITION;
    float3 WorldNormal : NORMAL;
};

VertexOut main(VertexIn vin)
{
    VertexOut vout;
    
    // Transform to world space
    float4 posW = mul(gWorld, float4(vin.Pos, 1.f));
    vout.WorldPos = posW.xyz;
    
    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.WorldNormal = mul((float3x3) gWorld, vin.Normal);

    // Transform to homogeneous clip space.
    vout.Pos = mul(gViewProj, posW);

    return vout;
}