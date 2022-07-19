cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
};

cbuffer cbPass : register(b1)
{
    float4x4	gView;
    float4x4	gInvView;
    float4x4	gProj;
    float4x4	gInvProj;
    float4x4	gViewProj;
    float4x4	gInvViewProj;
	float3		gCameraPosW;
    float		cbPerObjectPad1;
    float2		gRenderTargetSize;
    float2		gInvRenderTargetSize;
    float		gNearZ;
    float		gFarZ;
    float		gTotalTime;
    float		gDeltaTime;
};

struct VertexIn
{
	float3 pos : POSITION;
	float4 color : COLOR;
};

struct PixelIn
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

PixelIn VS(VertexIn vin)
{
	PixelIn vout;
	
	// Transform homogeneous to clip space.
    vout.pos = mul(float4(vin.pos, 1.0f), gWorld);
    vout.pos = mul(vout.pos, gViewProj);
	
	// Passing color to ps
	vout.color = vin.color;
	
	return vout;
}

float4 PS(PixelIn pin) : SV_TARGET
{
	return pin.color;
}
