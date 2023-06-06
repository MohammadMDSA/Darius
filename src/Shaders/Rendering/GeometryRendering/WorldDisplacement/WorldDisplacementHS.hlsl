#include "../../Common.hlsli"

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float3x3 gWorldIT;
};

struct VertexOut
{
    float3 LocalPos :   POSITION;
    float3 Normal :     NORMAL;
#ifndef NO_TANGENT_FRAME
    float4 Tangent :    TANGENT;
#endif
    float2 UV :         TEXCOORD0;
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

struct PatchTess
{
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VertexOut, 3> patch,
                        uint patchId : SV_PrimitiveID)
{
    PatchTess pt;
    float3 centerW = (patch[0].LocalPos + patch[1].LocalPos + patch[2].LocalPos) / 3;

    float d = distance(centerW, gCameraPosW);
    
    const float d0 = 10.0f;
    const float d1 = 20.0f;
    float tess = 4 * saturate((d1 - d) / (d1 - d0));
    tess = clamp(tess, 1.f, 4.f);
    
    // Uniform tessellation
    pt.EdgeTess[0] = tess;
    pt.EdgeTess[1] = tess;
    pt.EdgeTess[2] = tess;
    
    pt.InsideTess = tess;
    
    return pt;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(4.0f)]
[RootSignature(Renderer_RootSig)]
HullOut main(InputPatch<VertexOut, 3> p,
                uint i : SV_OutputControlPointID,
                uint patchId : SV_PrimitiveID)
{
    
    // A simple pass through hull shader
    HullOut hout;
    hout.LocalPos = p[i].LocalPos;
    hout.Normal = p[i].Normal;
#ifndef NO_TANGENT_FRAME    
    hout.Tangent = p[i].Tangent;
#endif
    hout.UV = p[i].UV;

    return hout;
}
