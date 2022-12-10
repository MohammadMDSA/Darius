#include "Common.hlsli"

cbuffer PSConstants : register(b0)
{
	float TextureLevel;
};

TextureCube<float3> radianceIBLTexture : register(t12);

struct VSOutput
{
	float4 position : SV_POSITION;
	float3 viewDir : TEXCOORD3;
};

[RootSignature(Renderer_RootSig)]
float4 main(VSOutput vsOutput) : SV_Target0
{
    float3 aa = radianceIBLTexture.SampleLevel(defaultSampler, vsOutput.viewDir, TextureLevel);
	return float4(aa, 1);
}