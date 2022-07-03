cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
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
	vout.pos = mul(float4(vin.pos, 1.0f), gWorldViewProj);
	
	// Passing color to ps
	vout.color = vin.color;
	
	return vout;
}

float4 PS(PixelIn pin) : SV_TARGET
{
	return pin.color;
}
