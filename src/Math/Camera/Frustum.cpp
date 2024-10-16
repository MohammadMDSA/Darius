//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "../pch.hpp"
#include "Frustum.hpp"
#include "Camera.hpp"

namespace Darius::Math::Camera
{

	void Frustum::ConstructPerspectiveFrustum(float HTan, float VTan, float NearClip, float FarClip)
	{
		const float NearX = HTan * NearClip;
		const float NearY = VTan * NearClip;
		const float FarX = HTan * FarClip;
		const float FarY = VTan * FarClip;

		// Define the frustum corners
		m_FrustumCorners[kNearLowerLeft] = Vector3(-NearX, -NearY, -NearClip);	// Near lower left
		m_FrustumCorners[kNearUpperLeft] = Vector3(-NearX, NearY, -NearClip);	// Near upper left
		m_FrustumCorners[kNearLowerRight] = Vector3(NearX, -NearY, -NearClip);	// Near lower right
		m_FrustumCorners[kNearUpperRight] = Vector3(NearX, NearY, -NearClip);	// Near upper right
		m_FrustumCorners[kFarLowerLeft] = Vector3(-FarX, -FarY, -FarClip);	// Far lower left
		m_FrustumCorners[kFarUpperLeft] = Vector3(-FarX, FarY, -FarClip);	// Far upper left
		m_FrustumCorners[kFarLowerRight] = Vector3(FarX, -FarY, -FarClip);	// Far lower right
		m_FrustumCorners[kFarUpperRight] = Vector3(FarX, FarY, -FarClip);	// Far upper right

		const float NHx = RecipSqrt(1.0f + HTan * HTan);
		const float NHz = -NHx * HTan;
		const float NVy = RecipSqrt(1.0f + VTan * VTan);
		const float NVz = -NVy * VTan;

		// Define the bounding planes
		m_FrustumPlanes[kNearPlane] = D_MATH_BOUNDS::BoundingPlane(0.0f, 0.0f, -1.0f, -NearClip);
		m_FrustumPlanes[kFarPlane] = D_MATH_BOUNDS::BoundingPlane(0.0f, 0.0f, 1.0f, FarClip);
		m_FrustumPlanes[kLeftPlane] = D_MATH_BOUNDS::BoundingPlane(NHx, 0.0f, NHz, 0.0f);
		m_FrustumPlanes[kRightPlane] = D_MATH_BOUNDS::BoundingPlane(-NHx, 0.0f, NHz, 0.0f);
		m_FrustumPlanes[kTopPlane] = D_MATH_BOUNDS::BoundingPlane(0.0f, -NVy, NVz, 0.0f);
		m_FrustumPlanes[kBottomPlane] = D_MATH_BOUNDS::BoundingPlane(0.0f, NVy, NVz, 0.0f);
	}

	void Frustum::ConstructOrthographicFrustum(float Left, float Right, float Top, float Bottom, float Front, float Back)
	{
		// Define the frustum corners
		m_FrustumCorners[kNearLowerLeft] = Vector3(Left, Bottom, Back);	// Near lower left
		m_FrustumCorners[kNearUpperLeft] = Vector3(Left, Top, Back);	// Near upper left
		m_FrustumCorners[kNearLowerRight] = Vector3(Right, Bottom, Back);	// Near lower right
		m_FrustumCorners[kNearUpperRight] = Vector3(Right, Top, Back);	// Near upper right
		m_FrustumCorners[kFarLowerLeft] = Vector3(Left, Bottom, Front);	// Far lower left
		m_FrustumCorners[kFarUpperLeft] = Vector3(Left, Top, Front);	// Far upper left
		m_FrustumCorners[kFarLowerRight] = Vector3(Right, Bottom, Front);	// Far lower right
		m_FrustumCorners[kFarUpperRight] = Vector3(Right, Top, Front);	// Far upper right

		// Define the bounding planes
		m_FrustumPlanes[kNearPlane] = D_MATH_BOUNDS::BoundingPlane(Vector3::Forward, Back);
		m_FrustumPlanes[kFarPlane] = D_MATH_BOUNDS::BoundingPlane(Vector3::Backward, -Front);
		m_FrustumPlanes[kLeftPlane] = D_MATH_BOUNDS::BoundingPlane(Vector3::Right, -Left);
		m_FrustumPlanes[kRightPlane] = D_MATH_BOUNDS::BoundingPlane(Vector3::Left, Right);
		m_FrustumPlanes[kTopPlane] = D_MATH_BOUNDS::BoundingPlane(Vector3::Down, -Bottom);
		m_FrustumPlanes[kBottomPlane] = D_MATH_BOUNDS::BoundingPlane(Vector3::Up, Top);
	}


	Frustum::Frustum(const Matrix4& ProjMat)
	{
		const float* ProjMatF = (const float*)&ProjMat;

		const float RcpXX = 1.0f / ProjMatF[0];
		const float RcpYY = 1.0f / ProjMatF[5];
		const float RcpZZ = 1.0f / ProjMatF[10];

		// Identify if the projection is perspective or orthographic by looking at the 4th row.
		if (ProjMatF[3] == 0.0f && ProjMatF[7] == 0.0f && ProjMatF[11] == 0.0f && ProjMatF[15] == 1.0f)
		{
			// Orthographic
			float Left = (-1.0f - ProjMatF[12]) * RcpXX;
			float Right = (1.0f - ProjMatF[12]) * RcpXX;
			float Top = (1.0f - ProjMatF[13]) * RcpYY;
			float Bottom = (-1.0f - ProjMatF[13]) * RcpYY;
			float Front = (0.0f - ProjMatF[14]) * RcpZZ;
			float Back = (1.0f - ProjMatF[14]) * RcpZZ;

			// Check for reverse Z here.  The bounding planes need to point into the frustum.
			if (Front < Back)
				ConstructOrthographicFrustum(Left, Right, Top, Bottom, Front, Back);
			else
				ConstructOrthographicFrustum(Left, Right, Top, Bottom, Back, Front);
		}
		else
		{
			// Perspective
			float NearClip, FarClip;

			if (RcpZZ > 0.0f)	// Reverse Z
			{
				FarClip = ProjMatF[14] * RcpZZ;
				NearClip = FarClip / (RcpZZ + 1.0f);
			}
			else
			{
				NearClip = ProjMatF[14] * RcpZZ;
				FarClip = NearClip / (RcpZZ + 1.0f);
			}

			ConstructPerspectiveFrustum(RcpXX, RcpYY, NearClip, FarClip);
		}
	}

	D_MATH_BOUNDS::AxisAlignedBox Frustum::GetAABB(float backwardsDepthBias) const
	{
		D_MATH_BOUNDS::AxisAlignedBox result;
		for (int corner = 0; corner < 8; corner++)
		{
			if (corner == 0)
				result = D_MATH_BOUNDS::AxisAlignedBox(m_FrustumCorners[0], m_FrustumCorners[0]);
			else
				result.AddPoint(m_FrustumCorners[corner]);
		}

		Vector3 depthBias = result.GetCenter() + Vector3::Backward * backwardsDepthBias;
		result.AddPoint(depthBias);

		return result;
	}

}