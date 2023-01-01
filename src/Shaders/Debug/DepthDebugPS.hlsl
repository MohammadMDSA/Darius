#include "../Common.hlsli"

struct VSOutput
{
	float4 pos    : SV_POSITION;
	float2 tex    : TEXCOORD;
};

Texture2D DebptTexture : register(t0);

[RootSignature(Renderer_RootSig)]
float4 main(VSOutput pin) : SV_Target
{
	return float4(DebptTexture.Sample(linearWrap, pin.tex).rrr, 1.f);
}