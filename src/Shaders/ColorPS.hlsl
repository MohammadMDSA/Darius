#include "Lighting.hlsli"

cbuffer cbPerObject : register(b0)
{
    float4 gColor;
};

struct PixelIn
{
	float4 pos : SV_POSITION;
};

[RootSignature(Renderer_RootSig)]
float4 main(PixelIn pin) : SV_TARGET
{
    return gColor;
}
