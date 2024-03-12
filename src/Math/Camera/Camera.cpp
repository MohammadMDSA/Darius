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
#include "Camera.hpp"

#include <cmath>

#include "Camera.sgenerated.hpp"

using namespace DirectX;

namespace Darius::Math::Camera
{

	Matrix4 Camera::CalculatePerspectiveProjectionMatrix(float verticalFoV, float aspectRatio, float nearClip, float farClip, bool reverseZ, bool infiniteZ)
	{
		float Y = 1.0f / std::tanf(verticalFoV * 0.5f);
		float X = Y * aspectRatio;

		float Q1, Q2;

		// ReverseZ puts far plane at Z=0 and near plane at Z=1.  This is never a bad idea, and it's
		// actually a great idea with F32 depth buffers to redistribute precision more evenly across
		// the entire range.  It requires clearing Z to 0.0f and using a GREATER variant depth test.
		// Some care must also be done to properly reconstruct linear W in a pixel shader from hyperbolic Z.
		if (reverseZ)
		{
			if (infiniteZ)
			{
				Q1 = 0.0f;
				Q2 = nearClip;
			}
			else
			{
				Q1 = nearClip / (farClip - nearClip);
				Q2 = Q1 * farClip;
			}
		}
		else
		{
			if (infiniteZ)
			{
				Q1 = -1.0f;
				Q2 = -nearClip;
			}
			else
			{
				Q1 = farClip / (nearClip - farClip);
				Q2 = Q1 * nearClip;
			}
		}

		return Matrix4(
			Vector4(X, 0.0f, 0.0f, 0.0f),
			Vector4(0.0f, Y, 0.0f, 0.0f),
			Vector4(0.0f, 0.0f, Q1, -1.0f),
			Vector4(0.0f, 0.0f, Q2, 0.0f)
		);
	}

	Matrix4 Camera::CalculateOrthographicProjectionMatrix(float orthographicSize, float aspectRatio, float nearClip, float farClip, float reverseZ)
	{
		auto orthoWidth = orthographicSize * 2;
		auto orthoHeigh = aspectRatio * orthoWidth;
		if (reverseZ)
		{
			return Matrix4(XMMatrixOrthographicRH(orthoWidth, orthoHeigh, farClip, nearClip));
		}
		else
		{
			return Matrix4(XMMatrixOrthographicRH(orthoWidth, orthoHeigh, nearClip, farClip));
		}
	}

	void Camera::UpdateProjMatrix(void)
	{
		if (mOrthographic)
		{
			SetProjMatrix(CalculateOrthographicProjectionMatrix(mOrthographicSize, mAspectRatio, mNearClip, mFarClip, mReverseZ));
		}
		else
		{
			SetProjMatrix(CalculatePerspectiveProjectionMatrix(mVerticalFoV, mAspectRatio, mNearClip, mFarClip, mReverseZ, mInfiniteZ));
		}
	}

	void Camera::CalculateSlicedFrustumes(D_CONTAINERS::DVector<float> const& ranges, D_CONTAINERS::DVector<Frustum>& slicedFrustums) const
	{
		slicedFrustums.reserve(ranges.size() + 1);

		float accumulatedRange = GetNearClip();

		for (int i = 0; i < (int)ranges.size() + 1; i++)
		{
			bool lastRange;
			float farRange;
			if (i == (int)ranges.size())
			{
				lastRange = true;
				farRange = mFarClip;
			}
			else
			{
				float currentRangeEnd = accumulatedRange + ranges.at(i);
				lastRange = currentRangeEnd >= mFarClip;
				farRange = lastRange ? mFarClip : currentRangeEnd;

			}

			Matrix4 projection = mOrthographic ?
				CalculateOrthographicProjectionMatrix(mOrthographicSize, mAspectRatio, accumulatedRange, farRange, mReverseZ) :
				CalculatePerspectiveProjectionMatrix(mVerticalFoV, mAspectRatio, accumulatedRange, farRange, mReverseZ, mInfiniteZ);

			Frustum vsFrust(projection);
			// Transform to world space
			slicedFrustums.push_back(m_CameraToWorld * vsFrust);

			if (lastRange)
				return;

			accumulatedRange += ranges.at(i);

		}
	}
}
