#define HLSL

#include "../../RaytracingHlslCompat.h"

// Common (static) samplers
SamplerState        defaultSampler  : register(s10);
SamplerState        cubeMapSampler  : register(s11);
SamplerState        linearWrap      : register(s12);

// GLOBALS /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

cbuffer cbPass : register(b0, space0)
{
    float4x4            g_View;
    float4x4            g_InvView;
    float4x4            g_Proj;
    float4x4            g_InvProj;
    float4x4            g_ViewProj;
    float4x4            g_InvViewProj;
    float4x4            g_InvViewProjEyeCenter;
    float4              g_FrustumPlanes[6];
    float4              g_ShadowTexelSize;
    float3              g_CameraPosW;
    float               c_bPerObjectPad1;
    float2              g_RenderTargetSize;
    float2              g_InvRenderTargetSize;
    float               g_NearZ;
    float               g_FarZ;
    float               g_TotalTime;
    float               g_DeltaTime;
    float4              g_AmbientLight;
    float               g_IBLRange;
    float               g_IBLBias;
    
};

RaytracingAccelerationStructure     g_RtScene               : register(t0, space0);
TextureCube<float3>                 g_RadianceIBLTexture    : register(t1, space0);
TextureCube<float3>                 g_IrradianceIBLTexture  : register(t2, space0);


// Locals //////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
// RayGen
RWTexture2D<float4> l_ColorOutput : register(u0, space1);

// Hit Group
cbuffer PerFrame : register(b0, space2)
{
    float3 A[3];
}

struct Ray
{
    float3 origin;
    float3 direction;
};

inline Ray GenerateCameraRay(uint2 index, in float3 cameraPosition, in float4x4 projectionToWorldWithCameraAtOrigin, float2 jitter = float2(0, 0))
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
    xy += jitter;
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    // Unproject the pixel coordinate into a world positon.
    float4 world = mul(float4(screenPos, 0, 1), projectionToWorldWithCameraAtOrigin);

    Ray ray;
    ray.origin = cameraPosition;
	// Since the camera's eye was at 0,0,0 in projectionToWorldWithCameraAtOrigin 
	// the world.xyz is the direction.
    ray.direction = normalize(world.xyz);

    return ray;
}

float3 LinearToSrgb(float3 c)
{
    // Based on http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
    float3 sq1 = sqrt(c);
    float3 sq2 = sqrt(sq1);
    float3 sq3 = sqrt(sq2);
    float3 srgb = 0.662002687 * sq1 + 0.684122060 * sq2 - 0.323583601 * sq3 - 0.0225411470 * c;
    return srgb;
}

float3 HitAttribute(float3 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attribs)
{
    float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
    
    return vertexAttribute[0] * barycentrics.x + vertexAttribute[1] * barycentrics.y + vertexAttribute[2] * barycentrics.z;
}

[shader("raygeneration")]
void MainRenderRayGen()
{
    uint3 launchIndex = DispatchRaysIndex();
    uint3 launchDim = DispatchRaysDimensions();

    float2 crd = float2(launchIndex.xy);
    float2 dims = float2(launchDim.xy);

    float2 d = ((crd / dims) * 2.f - 1.f);
    float aspectRatio = dims.x / dims.y;

    Ray ray = GenerateCameraRay(launchIndex.xy, g_CameraPosW, g_InvViewProjEyeCenter);
    
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;

    rayDesc.TMin = g_NearZ;
    rayDesc.TMax = g_FarZ;

    Payload payload;
    TraceRay(g_RtScene, 0 /*rayFlags*/, 0xFF, 0 /* ray index*/, 0, 0, rayDesc, payload);
    float3 col = LinearToSrgb(payload.color);
    l_ColorOutput[launchIndex.xy] = float4(col, 1);
    //gOutput[launchIndex.xy] = float4(1, 1, 1, 1);
}

[shader("miss")]
void MainRenderMiss(inout Payload payload)
{
    payload.color = g_RadianceIBLTexture.SampleLevel(cubeMapSampler, WorldRayDirection(), 0);
}

[shader("closesthit")]
void MainRenderCHS(inout Payload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.color = HitAttribute(A, attribs);
}