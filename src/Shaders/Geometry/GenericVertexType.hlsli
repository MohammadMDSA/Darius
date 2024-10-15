#ifndef __GENERIC_VERTEX_TYPE_HLSLI__
#define __GENERIC_VERTEX_TYPE_HLSLI__

#define HAS_UV0
#define HAS_UV1
#define HAS_UV2
#define HAS_UV3
#define ENABLE_SKINNING

struct GenericVSInputVertexLayout
{
	float3				LocalPosition :				POSITION;
	float3				Normal :					NORMAL;
	
#ifndef NO_TANGENT_FRAME
	float4				Tangent :					TANGENT;
#endif
	
#ifdef HAS_UV0

	float2				UV0 :						TEXCOORD0;

#ifdef HAS_UV1

	float2				UV1 :						TEXCOORD1;
	
#ifdef HAS_UV2

	float2				UV2 :						TEXCOORD2;
	
#ifdef HAS_UV3
	
	float2				UV3 :						TEXCOORD3;

#endif
#endif
#endif
#endif
	
#ifdef ENABLE_SKINNING
	uint4				JointIndices :				BLENDINDICES;
	float4				JointWeights :				BLENDWEIGHT;
#endif
};

struct GenericVSOutputVertexLayout
{
	float3				LocalPosition :				POSITION;
	float3				LocalNormal :				NORMAL;
	
#ifndef WORLD_DISPLACEMENT // World specifig data
	float4				ScreenNDC :					SV_POSITION;
	float3				WorldPosition :				POSITION;
	float3				WorldNormal :				NORMAL;
#endif
	
#ifndef NO_TANGENT_FRAME
	float4				Tangent :					TANGENT;
#endif
	
#ifdef HAS_UV0

	float2				UV0 :						TEXCOORD0;

#ifdef HAS_UV1

	float2				UV1 :						TEXCOORD1;
	
#ifdef HAS_UV2

	float2				UV2 :						TEXCOORD2;
	
#ifdef HAS_UV3
	
	float2				UV3 :						TEXCOORD3;

#endif
#endif
#endif
#endif
};

struct GenericPSInputVertexLayout
{
	float3				LocalPosition :				POSITION;
	float3				LocalNormal :				NORMAL;
	float4				ScreenNDC :					SV_POSITION;
	float3				WorldPosition :				POSITION;
	float3				WorldNormal :				NORMAL;
	
#ifndef NO_TANGENT_FRAME
	float4				Tangent :					TANGENT;
#endif
	
#ifdef HAS_UV0

	float2				UV0 :						TEXCOORD0;

#ifdef HAS_UV1

	float2				UV1 :						TEXCOORD1;
	
#ifdef HAS_UV2

	float2				UV2 :						TEXCOORD2;
	
#ifdef HAS_UV3
	
	float2				UV3 :						TEXCOORD3;

#endif
#endif
#endif
#endif
};

#endif