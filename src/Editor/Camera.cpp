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

#include "pch.hpp"
#include "Camera.hpp"

#include <Core/Input.hpp>
#include <Core/TimeManager/SystemTime.hpp>
#include <Utils/Log.hpp>

using namespace D_MATH;

namespace D_EDITOR
{
	FlyingFPSCamera::FlyingFPSCamera(D_MATH_CAMERA::Camera& camera, Vector3 worldUp) :
		CameraController(camera),
		mDirtyOrientation(true)
	{
		mWorldUp = Normalize(worldUp);
		mWorldNorth = Normalize(Cross(mWorldUp, Vector3(kXUnitVector)));
		mWorldEast = Cross(mWorldNorth, mWorldUp);

		mHorizontalLookSensitivity = 2.0f;
		mVerticalLookSensitivity = 2.0f;
		mMoveSpeed = 10.0f;
		mMouseSensitivityX = 1.f;
		mMouseSensitivityY = 1.f;

		mCurrentPitch = Sin(Dot(camera.GetForwardVec(), mWorldUp));

		Vector3 forward = Normalize(Cross(mWorldUp, camera.GetRightVec()));
		mCurrentHeading = ATan2(-Dot(forward, mWorldEast), Dot(forward, mWorldNorth));

		mFineMovement = false;
		mFineRotation = false;
		mMomentum = true;

		mLastYaw = 0.0f;
		mLastPitch = 0.0f;
		mLastForward = 0.0f;
		mLastStrafe = 0.0f;
		mLastAscent = 0.0f;
	}

	void FlyingFPSCamera::Update(float deltaTime)	
	{
		if (mDirtyOrientation)
		{
			auto rot = mTargetCamera.GetRotation();
			auto angles = rot.Angles();
			mCurrentPitch = angles.GetX();
			mCurrentHeading = angles.GetY();

			auto up = rot * Vector3::Up;

			if (up.GetY() < 0.f)
			{
				mCurrentPitch = DirectX::XM_PI - mCurrentPitch;
				mCurrentHeading += DirectX::XM_PI;
			}

			mDirtyOrientation = false;
		}

		float timeScale = 1.0f;
		(timeScale);
		if (D_INPUT::IsFirstPressed(D_INPUT::DigitalInput::KeyLshift))
			mFineMovement = !mFineMovement;

		float speedScale = (mFineMovement ? 0.1f : 1.0f) * timeScale;

		float yaw = 0.f;
		float pitch = 0.f;
		float forward = mMoveSpeed * speedScale * (
			(D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyW) ? deltaTime : 0.0f) +
			(D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyS) ? -deltaTime : 0.0f)
			);
		float strafe = mMoveSpeed * speedScale * (
			(D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyD) ? deltaTime : 0.0f) +
			(D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyA) ? -deltaTime : 0.0f)
			);
		float ascent = mMoveSpeed * speedScale * (
			(D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyE) ? deltaTime : 0.0f) +
			(D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyQ) ? -deltaTime : 0.0f)
			);

		if (mMomentum)
		{
			ApplyMomentum(mLastForward, forward, deltaTime);
			ApplyMomentum(mLastStrafe, strafe, deltaTime);
			ApplyMomentum(mLastAscent, ascent, deltaTime);
		}

		// don't apply momentum to mouse inputs
		yaw += D_INPUT::GetAnalogInput(D_INPUT::AnalogInput::MouseX) * mMouseSensitivityX;
		pitch = -D_INPUT::GetAnalogInput(D_INPUT::AnalogInput::MouseY) * mMouseSensitivityY;

		mCurrentPitch -= pitch;

		mCurrentHeading -= yaw;
		if (mCurrentHeading > DirectX::XM_PI)
			mCurrentHeading -= DirectX::XM_2PI;
		else if (mCurrentHeading <= -DirectX::XM_PI)
			mCurrentHeading += DirectX::XM_2PI;

		auto orientation = Matrix3(mWorldEast, mWorldUp, -mWorldNorth) * Matrix3::MakeYRotation(mCurrentHeading) * Matrix3::MakeXRotation(mCurrentPitch);

		Vector3 position = orientation * Vector3(strafe, ascent, -forward) + mTargetCamera.GetPosition();
		mTargetCamera.SetTransform(AffineTransform(orientation, position));
		mTargetCamera.Update();
	}

	void FlyingFPSCamera::SetHeadingPitchAndPosition(float heading, float pitch, const Vector3& position)
	{
		mCurrentHeading = heading;
		if (mCurrentHeading > DirectX::XM_PI)
			mCurrentHeading -= DirectX::XM_2PI;
		else if (mCurrentHeading <= -DirectX::XM_PI)
			mCurrentHeading += DirectX::XM_2PI;

		mCurrentPitch = pitch;
		mCurrentPitch = DirectX::XMMin(DirectX::XM_PIDIV2, mCurrentPitch);
		mCurrentPitch = DirectX::XMMax(-DirectX::XM_PIDIV2, mCurrentPitch);

		Matrix3 orientation =
			Matrix3(mWorldEast, mWorldUp, -mWorldNorth) *
			Matrix3::MakeYRotation(mCurrentHeading) *
			Matrix3::MakeXRotation(mCurrentPitch);

		mTargetCamera.SetTransform(AffineTransform(orientation, position));
		mTargetCamera.Update();
	}

	void CameraController::ApplyMomentum(float& oldValue, float& newValue, float deltaTime)
	{
		float blendedValue;
		if (Abs(newValue) > Abs(oldValue))
			blendedValue = Lerp(newValue, oldValue, Pow(0.6f, deltaTime * 60.0f));
		else
			blendedValue = Lerp(newValue, oldValue, Pow(0.8f, deltaTime * 60.0f));
		oldValue = blendedValue;
		newValue = blendedValue;
	}

	OrbitCamera::OrbitCamera(D_MATH_CAMERA::Camera& camera, D_MATH_BOUNDS::BoundingSphere focus, Vector3 worldUp) :
		CameraController(camera),
		mAdjusting(false),
		mTargetLocationDirty(false)
	{
		mModelBounds = focus;
		mWorldUp = Normalize(worldUp);

		mJoystickSensitivityX = 2.0f;
		mJoystickSensitivityY = 2.0f;

		mMouseSensitivityX = 1.f;
		mMouseSensitivityY = 1.f;
		mMouseSensitivityWheel = 1.f;

		mCurrentCloseness = 0.5f;

		mMomentum = true;

		mLastYaw = 0.0f;
		mLastPitch = 0.0f;
		mLastForward = 0.0f;

		mAdjustmentTime = 0.5f;
	}

	void OrbitCamera::SetTarget(Vector3 target)
	{
		if (mAdjusting)
			return;
		mAdjustmentTarget = ComputePositionFromTarget(target);
		mAdjusting = true;
		mAdjustmentStartTime = (float)D_TIME::SystemTime::GetCurrentSecond();
		mAdjustmentStartLocation = mTargetCamera.GetPosition();
	}

	void OrbitCamera::Update(float)
	{
		if (mAdjusting)
		{
			auto passedTime = (float)D_TIME::SystemTime::GetCurrentSecond() - mAdjustmentStartTime;
			if (mAdjustmentTime <= passedTime)
			{
				mAdjusting = false;
				mTargetCamera.SetPosition(mAdjustmentTarget);
				SetTargetLocationDirty();
			}
			else
			{
				auto normal = D_MATH::ExpoEaseOut(passedTime, 0, 1, mAdjustmentTime);
				auto location = D_MATH::Lerp(mAdjustmentTarget, mAdjustmentStartLocation, normal);
				mTargetCamera.SetPosition(location);

				mTargetCamera.Update();
				return;
			}
		}

		// don't apply momentum to mouse inputs
		float yaw = D_INPUT::GetAnalogInput(D_INPUT::AnalogInput::MouseX) * mMouseSensitivityX;
		float pitch = D_INPUT::GetAnalogInput(D_INPUT::AnalogInput::MouseY) * mMouseSensitivityY;
		float closeness = D_INPUT::GetAnalogInput(D_INPUT::AnalogInput::MouseScroll) * mMouseSensitivityWheel;

		auto rotationAngles = mTargetCamera.GetRotation().Angles();

		float mCurrentPitch = pitch + rotationAngles.GetX();
		mCurrentPitch = DirectX::XMMin(DirectX::XM_PIDIV2, mCurrentPitch);
		mCurrentPitch = DirectX::XMMax(-DirectX::XM_PIDIV2, mCurrentPitch);

		float mCurrentHeading = -yaw + rotationAngles.GetY();
		if (mCurrentHeading > DirectX::XM_PI)
			mCurrentHeading -= DirectX::XM_2PI;
		else if (mCurrentHeading <= -DirectX::XM_PI)
			mCurrentHeading += DirectX::XM_2PI;

		mCurrentCloseness += closeness;
		mCurrentCloseness = Clamp(mCurrentCloseness, 0.0f, 1.0f);

		Matrix3 orientation = Matrix3::MakeYRotation(mCurrentHeading) * Matrix3::MakeXRotation(mCurrentPitch);

		Vector3 position;

		if (mTargetLocationDirty)
		{
			position = mTargetCamera.GetPosition();

			mTargetLocation = -position + orientation.GetZ() * (mModelBounds.GetRadius() * Lerp(3.0f, 1.0f, mCurrentCloseness) + mTargetCamera.GetNearClip());

			mTargetLocationDirty = false;
		}
		else
		{
			position = -mTargetLocation + orientation.GetZ() * (mModelBounds.GetRadius() * Lerp(3.0f, 1.0f, mCurrentCloseness) + mTargetCamera.GetNearClip());
		}
		mTargetCamera.SetTransform(AffineTransform(orientation, position + mModelBounds.GetCenter()));

		mTargetCamera.Update();
	}


}