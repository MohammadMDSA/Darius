#include "../../Common.hlsli"

struct VSOutput
{
    float4 Pos : SV_Position;
    float2 SizeW : SIZE0;
};

struct GSOutput
{
    float4 Pos : SV_POSITION;
    float3 WorldPos : POSITION;
    float3 WorldNormal : NORMAL;
#ifndef NO_TANGENT_FRAME
    float4 Tangent : TANGENT;
#endif
    float2 UV : TEXCOORD0;
    
};

[RootSignature(Renderer_RootSig)]
[maxvertexcount(4)]
void main(point VSOutput gin[1],
        uint primID : SV_PrimitiveID,
        inout TriangleStream<GSOutput> triStream)
{
    float3 up = float3(0.f, 1.f, 0.f);
    float3 look = gCameraPosW - gin[0].Pos.xyz;
    look.y = 0.f;
    look = normalize(look);
    float3 right = cross(up, look);
    
    float halfWidth = 0.5f * gin[0].SizeW.x;
    float halfHeight = 0.5f * gin[0].SizeW.y;

    float4 v[4];
    v[0] = float4(gin[0].Pos.xyz + halfWidth * right - halfHeight * up, 1.f);
    v[1] = float4(gin[0].Pos.xyz + halfWidth * right + halfHeight * up, 1.f);
    v[2] = float4(gin[0].Pos.xyz - halfWidth * right - halfHeight * up, 1.f);
    v[3] = float4(gin[0].Pos.xyz - halfWidth * right + halfHeight * up, 1.f);
    
    
    float2 texC[4] =
    {
        float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
    };
	
    GSOutput gout;
    [unroll]
    for (int i = 0; i < 4; i++)
    {
        gout.Pos = mul(gViewProj, v[i]);
        gout.WorldPos = v[i].xyz;
        gout.WorldNormal = look;
#ifndef NO_TANGENT_FRAME
        gout.Tangent = float4(right, 1.f);
#endif
        gout.UV = texC[i];
        
        triStream.Append(gout);
    }
    triStream.RestartStrip();

}