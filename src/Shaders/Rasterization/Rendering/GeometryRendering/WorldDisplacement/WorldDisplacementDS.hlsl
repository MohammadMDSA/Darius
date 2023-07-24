#include "../../Common.hlsli"

struct DomainOut
{
    float4 Pos : SV_POSITION;
    float3 WorldPos : POSITION;
    float3 WorldNormal : NORMAL;
#ifndef NO_TANGENT_FRAME
    float4 Tangent : TANGENT;
#endif
    float2 UV : TEXCOORD0;
};

struct PatchTess
{
    float EdgeTess[3]   : SV_TessFactor;
    float InsideTess    : SV_InsideTessFactor;
};

struct HullOut
{
    float3 LocalPos :   POSITION;
    float3 Normal :     NORMAL;
#ifndef NO_TANGENT_FRAME
    float4 Tangent :    TANGENT;
#endif
    float2 UV :         TEXCOORD0;
};

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float3x3 gWorldIT;
}

cbuffer cbMaterial : register(b2)
{
    float4 gDiffuseAlbedo;
    float3 gFresnelR0;
    float3 gEmissive;
    float gMetallic;
    float gRoughness;
    float gDisplacementAmount;
    uint gTexStats;
}

Texture2D<float> texWorldDisplacement : register(t0);

#define ResolveParam(param) (uvw.x * tri[0].param + uvw.y * tri[1].param + uvw.z * tri[2].param)

[domain("tri")]
[RootSignature(Renderer_RootSig)]
DomainOut main(PatchTess patchTess,
                float3 uvw : SV_DomainLocation,
                const OutputPatch<HullOut, 3> tri)
{
    DomainOut dout;
    
    // Normal
    float3 normal = normalize(ResolveParam(Normal));
    float3x3 wit = (float3x3) gWorldIT;
    dout.WorldNormal = mul(wit, normal);
    
    dout.UV = ResolveParam(UV);
    
    float displacementNorm = texWorldDisplacement.SampleLevel(linearWrap, dout.UV, 0.f).r;
    float3 displacement = normal * (displacementNorm * gDisplacementAmount);
    float3 localPos = ResolveParam(LocalPos);
    localPos += displacement;
    dout.WorldPos = mul(gWorld, float4(localPos, 1.f)).xyz;
    dout.Pos = mul(gViewProj, float4(dout.WorldPos, 1.f));
    
#ifndef NO_TANGENT_FRAME
    float4 tangent = ResolveParam(Tangent);
    dout.Tangent = ResolveParam(Tangent);
#endif

    return dout;
}

#undef ResolveParam

