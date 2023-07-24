#include "TerrainCommon.hlsli"

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float3x3 gWorldIT;
};

cbuffer cbMaterial : register(b2)
{
    float  gDisplacementAmount;
};

Texture2D<float> texWorldDisplacement : register(t0);

#define ResolveParam(param) lerp(lerp(quad[0].param, quad[1].param, uv.x),lerp(quad[2].param, quad[3].param, uv.x),uv.y)

[domain("quad")]
[RootSignature(Renderer_RootSig)]
DomainOut main(PatchTess patchTess,
                float2 uv : SV_DomainLocation,
                const OutputPatch<HullOut, 4> quad)
{
    DomainOut dout;
    
    // Normal
    float3 normal = normalize(ResolveParam(Normal));
    float3x3 wit = (float3x3) gWorldIT;
    dout.WorldNormal = mul(wit, normal);
    
    // UV
    dout.UV = ResolveParam(UV);

    // World Pos    
    float displacementNorm = texWorldDisplacement.SampleLevel(linearWrap, dout.UV, 0.f).r;
    float3 displacement = normal * (displacementNorm * gDisplacementAmount);
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
