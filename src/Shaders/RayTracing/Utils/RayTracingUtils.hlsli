
#ifndef __RAYTRACING_UTILS_HLSLI__
#define __RAYTRACING_UTILS_HLSLI__

#include "../RaytracingHlslCompat.h"

#define WIREFRAME_THRESHOLD 0.001
	
// Retrieve attribute at a hit position interpolated from vertex attributes using the hit's barycentrics.
float3 HitAttribute(float3 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
	return vertexAttribute[0] +
attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

// Retrieve attribute at a hit position interpolated from vertex attributes using the hit's barycentrics.
float2 HitAttribute(float2 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
	return vertexAttribute[0] +
    attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
    attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}
		
bool IsOnTriangleEdge(BuiltInTriangleIntersectionAttributes attr)
{
	return attr.barycentrics.x < WIREFRAME_THRESHOLD ||
		attr.barycentrics.y < WIREFRAME_THRESHOLD ||
		(1.f - attr.barycentrics.y - attr.barycentrics.x) < WIREFRAME_THRESHOLD;
}

#endif
