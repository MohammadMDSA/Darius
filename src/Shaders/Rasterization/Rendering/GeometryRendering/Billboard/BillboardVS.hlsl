#include "../../Common.hlsli"

cbuffer cbPerObject : register(b0)
{
    float4x4 world;
    float2 size;
};

struct VSOutput
{
    float4 Pos          : SV_Position;
    float2 SizeW         : SIZE0;
};

[RootSignature(Renderer_RootSig)]
VSOutput main(uint VertID : SV_VertexID)
{
    VSOutput vout;
    vout.Pos = mul(world, float4(0.f, 0.f, 0.f, 1.f));
    vout.SizeW = size;
    return vout;
}
