#define HLSL

#include "./RaytracingHlslCompat.h"

RaytracingAccelerationStructure gRtScene : register(t0);
RWTexture2D<float4> gOutput : register(u0);

cbuffer PerFrame : register(b1)
{
    float3 A[3];
}

cbuffer cbPass : register(b0)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float4x4 gInvViewProjEyeCenter;
    float4 gFrustumPlanes[6];
    float4 gShadowTexelSize;
    float3 gCameraPosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;
    float IBLRange;
    float IBLBias;
    
};

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

float3 HitAttribute(float3 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
        attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

[shader("raygeneration")]
void rayGen()
{
    uint3 launchIndex = DispatchRaysIndex();
    uint3 launchDim = DispatchRaysDimensions();

    float2 crd = float2(launchIndex.xy);
    float2 dims = float2(launchDim.xy);

    float2 d = ((crd / dims) * 2.f - 1.f);
    float aspectRatio = dims.x / dims.y;

    Ray ray = GenerateCameraRay(launchIndex.xy, gCameraPosW, gInvViewProjEyeCenter);
    
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;

    rayDesc.TMin = gNearZ;
    rayDesc.TMax = gFarZ;

    Payload payload;
    TraceRay(gRtScene, 0 /*rayFlags*/, 0xFF, 0 /* ray index*/, 0, 0, rayDesc, payload);
    float3 col = LinearToSrgb(payload.color);
    gOutput[launchIndex.xy] = float4(col, 1);
    //gOutput[launchIndex.xy] = float4(1, 1, 1, 1);
}

[shader("miss")]
void miss(inout Payload payload)
{
    payload.color = float3(0.4, 0.6, 0.2);
}

[shader("closesthit")]
void chs(inout Payload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.color = HitAttribute(A, attribs);
}