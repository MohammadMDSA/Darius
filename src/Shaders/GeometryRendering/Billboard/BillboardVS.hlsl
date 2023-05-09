#include "../Common.hlsli"

struct VSOutput
{
    float4 pos          : SV_Position;
};
    
[RootSignature(Renderer_RootSig)]
VSOutput main(uint VertID : SV_VertexID)
{
    VSOutput vout;
    vout.pos = float4(5.f, 0.f, 0.f, 0.f);
    
    return vout;
}
