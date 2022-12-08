cbuffer VSConstants : register(b0)
{
	float4x4 ProjInverse;
	float3x3 ViewInverse;
};

struct VSOutput
{
	float4 position : SV_POSITION;
	float3 viewDir : TEXCOORD3;
};

VSOutput main(uint VertID : SV_VertexID)
{
    float2 ScreenUV = float2(uint2(VertID, VertID << 1) & 2);
    float4 ProjectedPos = float4(lerp(float2(-1, 1), float2(1, -1), ScreenUV), 0, 1);
    float4 PosViewSpace = mul(ProjInverse, ProjectedPos);

    VSOutput vsOutput;
    vsOutput.position = ProjectedPos;
    vsOutput.viewDir = mul(ViewInverse, PosViewSpace.xyz / PosViewSpace.w);

    return vsOutput;
}