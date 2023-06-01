#include "TerrainCommon.hlsli"

[RootSignature(Renderer_RootSig)]
VertexOut main(VertexIn vin)
{
    VertexOut vout;
    vout.Pos = vin.Pos;
    vout.Normal = vin.Normal;
    
#ifndef NO_TANGENT_FRAME
    vout.Tangent = vin.Tangent;
#endif
    
    vout.UV = vin.UV;
    
    return vout;
}