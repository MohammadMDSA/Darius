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

    void Camera::UpdateProjMatrix(void)
    {
        if (mOrthographic)
        {
            Matrix4 projection;
            auto orthoWidth = mOrthographicSize * 2;
            auto orthoHeigh = mAspectRatio * orthoWidth;
            if (mReverseZ)
            {
                SetProjMatrix(Matrix4(XMMatrixOrthographicRH(mOrthographicSize, orthoHeigh, mFarClip, mNearClip)));
            }
            else
            {
                SetProjMatrix(Matrix4(XMMatrixOrthographicRH(orthoWidth, orthoHeigh, mNearClip, mFarClip)));
            }
        }
        else
        {
            float Y = 1.0f / std::tanf(mVerticalFoV * 0.5f);
            float X = Y * mAspectRatio;

            float Q1, Q2;

            // ReverseZ puts far plane at Z=0 and near plane at Z=1.  This is never a bad idea, and it's
            // actually a great idea with F32 depth buffers to redistribute precision more evenly across
            // the entire range.  It requires clearing Z to 0.0f and using a GREATER variant depth test.
            // Some care must also be done to properly reconstruct linear W in a pixel shader from hyperbolic Z.
            if (mReverseZ)
            {
                if (mInfiniteZ)
                {
                    Q1 = 0.0f;
                    Q2 = mNearClip;
                }
                else
                {
                    Q1 = mNearClip / (mFarClip - mNearClip);
                    Q2 = Q1 * mFarClip;
                }
            }
            else
            {
                if (mInfiniteZ)
                {
                    Q1 = -1.0f;
                    Q2 = -mNearClip;
                }
                else
                {
                    Q1 = mFarClip / (mNearClip - mFarClip);
                    Q2 = Q1 * mNearClip;
                }
            }

            SetProjMatrix(Matrix4(
                Vector4(X, 0.0f, 0.0f, 0.0f),
                Vector4(0.0f, Y, 0.0f, 0.0f),
                Vector4(0.0f, 0.0f, Q1, -1.0f),
                Vector4(0.0f, 0.0f, Q2, 0.0f)
            ));
        }
    }
}
