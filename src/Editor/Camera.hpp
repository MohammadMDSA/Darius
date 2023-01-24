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

#pragma once

#include <Math/VectorMath.hpp>
#include <Math/Bounds/BoundingSphere.hpp>
#include <Math/Camera/Camera.hpp>

#ifndef D_EDITOR
#define D_EDITOR Darius::Editor
#endif // !D_EDITOR

namespace Darius::Editor
{
    class CameraController
    {
    public:
        // Assumes worldUp is not the X basis vector
        CameraController(D_MATH_CAMERA::Camera& camera) : mTargetCamera(camera) {}
        virtual ~CameraController() {}
        virtual void Update(float dt) = 0;

        // Helper function
        static void ApplyMomentum(float& oldValue, float& newValue, float deltaTime);

    protected:
        D_MATH_CAMERA::Camera& mTargetCamera;

    private:
        CameraController& operator=(const CameraController&) { return *this; }
    };

    class FlyingFPSCamera : public CameraController
    {
    public:
        FlyingFPSCamera(D_MATH_CAMERA::Camera& camera, D_MATH::Vector3 worldUp);

        virtual void Update(float dt) override;

        void SlowMovement(bool enable) { mFineMovement = enable; }
        void SlowRotation(bool enable) { mFineRotation = enable; }

        void EnableMomentum(bool enable) { mMomentum = enable; }

        void SetHeadingPitchAndPosition(float heading, float pitch, const D_MATH::Vector3& position);

        INLINE void SetOrientationDirty() { mDirtyOrientation = true; mLastForward = mLastStrafe = mLastAscent = 0.f; }

    private:
        FlyingFPSCamera& operator=(const FlyingFPSCamera&) { return *this; }
        D_MATH::Vector3 mWorldUp;
        D_MATH::Vector3 mWorldNorth;
        D_MATH::Vector3 mWorldEast;

        float mHorizontalLookSensitivity;
        float mVerticalLookSensitivity;
        float mMoveSpeed;
        float mStrafeSpeed;
        float mMouseSensitivityX;
        float mMouseSensitivityY;

        float mCurrentHeading;
        float mCurrentPitch;

        bool mFineMovement;
        bool mFineRotation;
        bool mMomentum;
        bool mDirtyOrientation;

        float mLastYaw;
        float mLastPitch;
        float mLastForward;
        float mLastStrafe;
        float mLastAscent;
    };

    class OrbitCamera : public CameraController
    {
    public:
        OrbitCamera(D_MATH_CAMERA::Camera& camera,
            D_MATH_BOUNDS::BoundingSphere focus,
            D_MATH::Vector3 upVec = D_MATH::Vector3(D_MATH::kYUnitVector));

        virtual void Update(float dt) override;

        void EnableMomentum(bool enable) { mMomentum = enable; }
        void SetTarget(D_MATH::Vector3 target);

        INLINE void SetTargetLocationDirty() { mTargetLocationDirty = true; }
        INLINE bool IsAdjusting() { return mAdjusting; }
        INLINE float GetCurrentCloseness() const { return mCurrentCloseness; }

    private:
        INLINE D_MATH::Vector3 ComputeTargetFromPosition(D_MATH::Vector3 pos)
        {
            D_MATH::Matrix3 orientation = D_MATH::Matrix3(XMMatrixRotationQuaternion(mTargetCamera.GetRotation()));

            return -pos + orientation.GetZ() * (mModelBounds.GetRadius() * D_MATH::Lerp(3.0f, 1.0f, mCurrentCloseness) + mTargetCamera.GetNearClip());
        }

        INLINE D_MATH::Vector3 ComputePositionFromTarget(D_MATH::Vector3 target)
        {
            D_MATH::Matrix3 orientation = D_MATH::Matrix3(XMMatrixRotationQuaternion(mTargetCamera.GetRotation()));

            return target + orientation.GetZ() * (mModelBounds.GetRadius() * D_MATH::Lerp(3.0f, 1.0f, mCurrentCloseness) + mTargetCamera.GetNearClip());
        }

        OrbitCamera& operator=(const OrbitCamera&) { return *this; }

        D_MATH_BOUNDS::BoundingSphere mModelBounds;
        D_MATH::Vector3 mWorldUp;

        D_MATH::Vector3 mTargetLocation;
        D_MATH::Vector3 mAdjustmentTarget;
        D_MATH::Vector3 mAdjustmentStartLocation;

        float mJoystickSensitivityX;
        float mJoystickSensitivityY;

        float mMouseSensitivityX;
        float mMouseSensitivityY;
        float mMouseSensitivityWheel;

        float mCurrentCloseness;
        float mLastYaw;
        float mLastPitch;
        float mLastForward;

        float mAdjustmentTime;
        float mAdjustmentStartTime = -1;

        bool mMomentum;
        bool mTargetLocationDirty;

        bool mAdjusting;
    };
}