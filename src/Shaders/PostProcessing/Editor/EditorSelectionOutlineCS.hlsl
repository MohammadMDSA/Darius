#include "../PostEffectsRS.hlsli"

Texture2D<uint2>    StencilBuffer       : register(t0);
Texture2D<float3>   InColorBuffer       : register(t1);
RWTexture2D<float3> OutputColor         : register(u0);

cbuffer cb0 : register(b1)
{
    float2  RcpBufferDim;
    float   OutlineWidth;
    uint    StencilRef;
    float3  OutlineColor;
    float   Threshold;
}

float2 STtoUV(float2 ST)
{
    return (ST + 0.5) * RcpBufferDim;
}

[RootSignature(PostEffects_RootSig)]
[numthreads(8, 8, 1)]
void main(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
    float2 uv = STtoUV(DTid.xy);
    
    float3 referenceColor = InColorBuffer[DTid.xy];
    
    float2 scaledOutline = OutlineWidth * RcpBufferDim;
    
    uint depthx1 = StencilBuffer[DTid.xy + int2(0, OutlineWidth)].y;
    uint depthx2 = StencilBuffer[DTid.xy + int2(0, -OutlineWidth)].y;
    uint depthy1 = StencilBuffer[DTid.xy + int2(OutlineWidth, 0)].y;
    uint depthy2 = StencilBuffer[DTid.xy + int2(-OutlineWidth, 0)].y;

    float dx = floor(abs((depthx1 - depthx2) * Threshold));
    float dy = floor(abs((depthy1 - depthy2) * Threshold));
    
    float outlineStrength = saturate(dx + dy);
        
    OutputColor[DTid.xy] = lerp(referenceColor, OutlineColor, outlineStrength);

}