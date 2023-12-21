#include "../PostEffectsRS.hlsli"

Texture2D<float>    DepthBuffer         : register(t0);
Texture2D<float>    CustomDepthBuffer   : register(t1);
Texture2D<uint2>    StencilBuffer       : register(t2);
Texture2D<float3>   InColorBuffer       : register(t3);
RWTexture2D<float3> OutputColor         : register(u0);

cbuffer cb0 : register(b1)
{
    float2  RcpBufferDim;
    float   OutlineWidth;
    uint    StencilRef;
    float3  OutlineColor;
    float   Threshold;
    float3  CoveredOutlineColor;
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
    
    //if (StencilBuffer[DTid.xy] != StencilRef)
    //{
    //    OutputColor[DTid.xy] = referenceColor;
    //    return;
    //}
    
    float2 scaledOutline = OutlineWidth * RcpBufferDim;
    
    //float2 scaledOutlineX = float2(scaledOutline.x, 0);
    //float2 scaledOutlineY = float2(0, scaledOutline.y);
    
    //float2 coordx1 = uv + scaledOutlineX;
    //float2 coordx2 = uv - scaledOutlineX;
    //float2 coordy1 = uv + scaledOutlineY;
    //float2 coordy2 = uv - scaledOutlineY;
    
    uint depthx1 = StencilBuffer[DTid.xy + int2(0, OutlineWidth)].y;
    uint depthx2 = StencilBuffer[DTid.xy + int2(0, -OutlineWidth)].y;
    uint depthy1 = StencilBuffer[DTid.xy + int2(OutlineWidth, 0)].y;
    uint depthy2 = StencilBuffer[DTid.xy + int2(-OutlineWidth, 0)].y;
    //float depthx1 = CustomDepthBuffer.SampleLevel(LinearSamplerBorderBlack, coordx1, 0.f);
    //float depthx2 = CustomDepthBuffer.SampleLevel(LinearSamplerBorderBlack, coordx2, 0.f);
    //float depthy1 = CustomDepthBuffer.SampleLevel(LinearSamplerBorderBlack, coordy1, 0.f);
    //float depthy2 = CustomDepthBuffer.SampleLevel(LinearSamplerBorderBlack, coordy2, 0.f);

    float dx = floor(abs((depthx1 - depthx2) * Threshold));
    float dy = floor(abs((depthy1 - depthy2) * Threshold));
    
    float outlineStrength = saturate(dx + dy);
    
    bool visible = DepthBuffer[DTid.xy] <= CustomDepthBuffer[DTid.xy];
    
    float3 outlineCol = visible ? OutlineColor : CoveredOutlineColor;
    
    OutputColor[DTid.xy] = lerp(referenceColor, outlineCol, outlineStrength);

}