#include "Common.hlsli"

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float3x3 gWorldIT;
};

struct VertexIn
{
	float3 pos : POSITION;
};

struct PixelIn
{
	float4 pos : SV_POSITION;
};

PixelIn main(VertexIn vin)
{
	PixelIn vout;
	
	// Transform homogeneous to clip space.
    vout.pos = mul(gWorld, float4(vin.pos, 1.0f));
    vout.pos = mul(gViewProj, vout.pos);
	
	return vout;
}
