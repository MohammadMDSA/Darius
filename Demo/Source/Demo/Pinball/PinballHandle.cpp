#include <pch.hpp>
#include "PinballHandle.hpp"

#include <Core/Input.hpp>
#include <ResourceManager/Resource.hpp>

#ifdef _D_EDITOR
#include <Scene/Utils/GameObjectDragDropPayload.hpp>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "PinballHandle.sgenerated.hpp"

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(PinballHandle);

	PinballHandle::PinballHandle() :
		Super()
	{ }

	PinballHandle::PinballHandle(D_CORE::Uuid const& uuid) :
		Super(uuid)
	{ }

	void PinballHandle::Start()
	{

	}

	void PinballHandle::Update(float deltaTime)
	{
		if (mCurrentState == MovementState::Up)
		{
			if (D_INPUT::IsReleased((D_INPUT::DigitalInput)mKeyCode))
			{
				SetGoingDown();
			}
		}
		else if (mCurrentState == MovementState::Down)
		{
			if (D_INPUT::IsPressed((D_INPUT::DigitalInput)mKeyCode))
			{
				SetGoingUp();
			}
		}
		else
		{
			mTimeInMovement += deltaTime;

			float degDiff;
			float sourceValue;

			if (mCurrentState == MovementState::GoingUp)
			{
				degDiff = mRotationAmountDeg;
				sourceValue = 0;
			}
			else
			{
				degDiff = -mRotationAmountDeg;
				sourceValue = mRotationAmountDeg;
			}

			float currentRotDeg;
			if (mTimeInMovement >= mMovementTime)
			{

				currentRotDeg = sourceValue + degDiff;

				if (mCurrentState == MovementState::GoingUp)
				{
					mCurrentState = MovementState::Up;
				}
				else
				{
					mCurrentState = MovementState::Down;
				}
			}
			else
				currentRotDeg = D_MATH::ExpoEaseIn(mTimeInMovement, sourceValue, degDiff, mMovementTime);

			auto totalRot = D_MATH::Quaternion(mAxis, D_MATH::Deg2Rad(currentRotDeg));
			GetTransform()->SetRotation(totalRot);
		}
	}


	void PinballHandle::SetGoingUp()
	{
		if (mCurrentState != MovementState::Down)
			return;

		mCurrentState = MovementState::GoingUp;
		mTimeInMovement = 0.f;
	}

	void PinballHandle::SetGoingDown()
	{
		if (mCurrentState != MovementState::Up)
			return;

		mCurrentState = MovementState::GoingDown;
		mTimeInMovement = 0.f;
	}

#ifdef _D_EDITOR
	bool PinballHandle::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();


		{
			D_H_DETAILS_DRAW_PROPERTY("Movement Time");
			ImGui::DragFloat("##MovementTime", &mMovementTime, 0.1f, 0.f);
		}

		{
			D_H_DETAILS_DRAW_PROPERTY("Rotation Amount");
			ImGui::DragFloat("##RotationAmount", &mRotationAmountDeg, 1.f);
		}

		{
			D_H_DETAILS_DRAW_PROPERTY("Axis");
			D_MATH::DrawDetails(mAxis);
		}

		{
			D_H_DETAILS_DRAW_PROPERTY("Key Code");
			static uint8_t step = 1;
			ImGui::InputScalar("##KeyCode", ImGuiDataType_U8, &mKeyCode, &step);
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

}
