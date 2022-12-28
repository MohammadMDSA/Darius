#include "Common.hlsli"

struct VSOutput
{
	float4 pos    : SV_POSITION;
	float2 tex    : TEXCOORD;
};

[RootSignature(Renderer_RootSig)]
VSOutput main(int VertID : SV_VertexID)
{
	VSOutput vout;
	vout.tex = float2(uint2(VertID, VertID << 1) & 2);
	vout.pos = vout.tex * 2.f - 1.f;

	return vout;
}