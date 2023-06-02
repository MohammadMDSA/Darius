#include "TerrainCommon.hlsli"

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float3x3 gWorldIT;
};

PatchTess ConstantHS(InputPatch<VertexOut, 4> patch,
                        uint patchId : SV_PrimitiveID)
{
    PatchTess pt;
    float3 centerL = 0.25f * (patch[0].Pos + patch[1].Pos + patch[2].Pos + patch[3].Pos);
    float3 centerW = mul(gWorld, float4(centerL, 1.f)).xyz;

    float d = distance(centerW, gCameraPosW);
    
    // Tessellate the patch based on distance from the eye such that
	// the tessellation is 0 if d >= d1 and 64 if d <= d0.  The interval
	// [d0, d1] defines the range we tessellate in.
	
    const float d0 = 600.0f;
    const float d1 = 1000.0f;
    float tess = 64 * saturate((d1 - d) / (d1 - d0));
    tess = clamp(tess, 1.f, 64.f);
    
    // Uniform tessellation
    pt.EdgeTess[0] = tess;
    pt.EdgeTess[1] = tess;
    pt.EdgeTess[2] = tess;
    pt.EdgeTess[3] = tess;
    
    pt.InsideTess[0] = tess;
    pt.InsideTess[1] = tess;
    
    return pt;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
[RootSignature(Renderer_RootSig)]
HullOut main(InputPatch<VertexOut, 4> p,
                uint i : SV_OutputControlPointID,
                uint patchId : SV_PrimitiveID)
{
    
    // A simple pass through hull shader
    
    HullOut hout;
    hout.Pos = p[i].Pos;
    hout.Normal = p[i].Normal;
#ifndef NO_TANGENT_FRAME    
    hout.Tangent = p[i].Tangent;
#endif
    hout.UV = p[i].UV;

    return hout;
}
