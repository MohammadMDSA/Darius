
#ifndef __RAYTRACING_COMMON_HLSLI__
#define __RAYTRACING_COMMON_HLSLI__

#include "RayTracingHlslCompat.h"

// Generate camera's forward direction ray
inline float3 GenerateForwardCameraRayDirection(in float4x4 projectionToWorldWithCameraAtOrigin)
{
	float2 screenPos = float2(0, 0);
	
	// Unproject the pixel coordinate into a world positon.
	float4 world = mul(float4(screenPos, 0, 1), projectionToWorldWithCameraAtOrigin);
	return normalize(world.xyz);
}

inline Ray GenerateForwardCameraRay(in float3 cameraPosition, in float4x4 projectionToWorldWithCameraAtOrigin)
{
    float2 screenPos = float2(0, 0);

    // Unproject the pixel coordinate into a world positon.
    float4 world = mul(float4(screenPos, 0, 1), projectionToWorldWithCameraAtOrigin);

    Ray ray;
    ray.Origin = cameraPosition;
    // Since the camera's eye was at 0,0,0 in projectionToWorldWithCameraAtOrigin 
    // the world.xyz is the direction.
    ray.Direction = normalize(world.xyz);

    return ray;
}

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
    ray.Origin = cameraPosition;
	// Since the camera's eye was at 0,0,0 in projectionToWorldWithCameraAtOrigin 
	// the world.xyz is the direction.
    ray.Direction = normalize(world.xyz);

    return ray;
}
    
// Retrieve hit world position.
float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

#endif
