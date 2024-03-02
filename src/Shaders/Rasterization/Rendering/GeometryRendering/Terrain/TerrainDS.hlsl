#include "TerrainCommon.hlsli"

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float3x3 gWorldIT;
};

cbuffer cbMaterial : register(b2)
{
    float  gDisplacementAmount;
    float2 gDisplacementTexInvSize;
};

Texture2D<float> texWorldDisplacement : register(t0);

#define ResolveParam(param) lerp(lerp(quad[0].param, quad[1].param, uv.x),lerp(quad[2].param, quad[3].param, uv.x),uv.y)

float getHeight(float2 uv, SamplerState state)
{
    return texWorldDisplacement.SampleLevel(state, uv, 0.f).r;
}

[domain("quad")]
[RootSignature(Renderer_RootSig)]
DomainOut main(PatchTess patchTess,
                float2 uv : SV_DomainLocation,
                const OutputPatch<HullOut, 4> quad)
{
    DomainOut dout;
    
    // UV
    dout.UV = ResolveParam(UV);

    // Normal
    float hL = getHeight(dout.UV - gDisplacementTexInvSize, linearClamp);
    float hR = getHeight(dout.UV + gDisplacementTexInvSize, linearClamp);
    float hD = getHeight(dout.UV - gDisplacementTexInvSize, linearClamp);
    float hU = getHeight(dout.UV + gDisplacementTexInvSize, linearClamp);
    // Deduce terrain normal
    float3 normal; // = normalize(ResolveParam(Normal));
    normal.x = (hL - hR) * gDisplacementAmount;
    normal.z = (hD - hU) * gDisplacementAmount;
    normal.y = 2.0;
    normal = normalize(normal);
    
    float3x3 wit = (float3x3) gWorldIT;
    dout.WorldNormal = mul(wit, normal);
    
    // World Pos    
    float displacementNorm = getHeight(dout.UV, linearWrap);
    float3 displacement = float3(0.f, displacementNorm * gDisplacementAmount, 0.f);
    float3 posL = ResolveParam(Pos);
    posL += displacement;
    dout.WorldPos = mul(gWorld, float4(posL, 1.f)).xyz;
    dout.Pos = mul(gViewProj, float4(dout.WorldPos, 1.f));

#ifndef NO_TANGENT_FRAME
    float4 tangent = ResolveParam(Tangent);
    dout.Tangent = float4(mul(gWorldIT, tangent.xyz), tangent.w);
#endif

    return dout;
}

#undef ResolveParam
