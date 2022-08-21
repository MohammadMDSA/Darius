#include "Lighting.hlsli"

cbuffer cbPerObject : register(b0)
{
    float4 gColor;
};

struct PixelIn
{
	float4 pos : SV_POSITION;
};

float4 PS(PixelIn pin) : SV_TARGET
{
    return gColor;
}
