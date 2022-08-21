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
	float3 pos : POSITION;
};

struct PixelIn
{
	float4 pos : SV_POSITION;
};

PixelIn VS(VertexIn vin)
{
	PixelIn vout;
	
	// Transform homogeneous to clip space.
    vout.pos = mul(gWorld, float4(vin.pos, 1.0f));
    vout.pos = mul(gViewProj, vout.pos);
	
	return vout;
}
