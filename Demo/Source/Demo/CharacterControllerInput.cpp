#include <pch.hpp>
#include "CharacterControllerInput.hpp"

#include <Core/Input.hpp>
#include <Physics/Resources/PhysicsMaterialResource.hpp>
#include <ResourceManager/Resource.hpp>

#ifdef _D_EDITOR
#include <Scene/Utils/GameObjectDragDropPayload.hpp>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "CharacterControllerInput.sgenerated.hpp"

using namespace D_MATH;

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(CharacterControllerInput);

	CharacterControllerInput::CharacterControllerInput() :
		D_ECS_COMP::BehaviourComponent(),
		mSpeed(10.f),
		mMouseSpeed(0.001f),
		mJumpSpeed(1.f),
		mUseMouseKeyboard(true),
		mGamepadLookSens(1.f),
		mLookPitch(0.f)
	{ }

	CharacterControllerInput::CharacterControllerInput(D_CORE::Uuid const& uuid) :
		D_ECS_COMP::BehaviourComponent(uuid),
		mSpeed(10.f),
		mMouseSpeed(0.001f),
		mJumpSpeed(1.f),
		mUseMouseKeyboard(true),
		mGamepadLookSens(1.f),
		mLookPitch(0.f)
	{ }

	void CharacterControllerInput::Start()
	{
		mController = GetGameObject()->GetComponent<D_PHYSICS::CharacterControllerComponent>();
	}

	void CharacterControllerInput::OnDestroy()
	{ }

	void CharacterControllerInput::Update(float deltaTime)
	{

		Vector3 movement = Vector3::Zero;

		if(mUseMouseKeyboard)
		{
			if(D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyW))
				movement += Vector3::Forward;
			if(D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyS))
				movement += Vector3::Backward;
			if(D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyA))
				movement += Vector3::Left;
			if(D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyD))
				movement += Vector3::Right;
			movement.Normalize();
		}
		else
		{
			movement += D_INPUT::GetAnalogInput(D_INPUT::AnalogInput::LeftStickY) * Vector3::Forward;
			movement += D_INPUT::GetAnalogInput(D_INPUT::AnalogInput::LeftStickX) * Vector3::Right;
		}


		float horizontal = 0;
		float vertical = 0;

		if(mUseMouseKeyboard)
		{
			horizontal += D_INPUT::GetAnalogInput(D_INPUT::AnalogInput::MouseX) * mMouseSpeed;
			vertical += D_INPUT::GetAnalogInput(D_INPUT::AnalogInput::MouseY) * mMouseSpeed;
		}
		else
		{
			horizontal += D_INPUT::GetTimeCorrectedAnalogInput(D_INPUT::AnalogInput::RightStickX) * mGamepadLookSens;
			vertical += D_INPUT::GetTimeCorrectedAnalogInput(D_INPUT::AnalogInput::RightStickY) * mGamepadLookSens;
		}

		mLookPitch += vertical;
		mLookPitch = D_MATH::Clamp(mLookPitch, -89.9f, 89.9f);

		D_LOG_DEBUG(mLookPitch);

		auto rot = Quaternion(Vector3::Up, D_MATH::Deg2Rad(-horizontal));
		auto lookRot = Quaternion(Vector3::Right, D_MATH::Deg2Rad(mLookPitch));

		auto trans = GetTransform();

		if(mUpperBody.IsValid())
			mUpperBody->GetTransform()->SetLocalRotation(lookRot);

		auto newRot = trans->GetRotation() * rot;
		trans->SetRotation(newRot);

		if(mController.IsValid())
			mController->Move((newRot * movement) * mSpeed * deltaTime);

		if(mUseMouseKeyboard)
		{
			if(D_INPUT::IsFirstPressed(D_INPUT::DigitalInput::KeySpace))
				mController->Jump(mJumpSpeed);
		}
		else
		{
			if(D_INPUT::IsFirstPressed(D_INPUT::DigitalInput::AButton))
				mController->Jump(mJumpSpeed);
		}
	}

#ifdef _D_EDITOR
	bool CharacterControllerInput::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();


		// Speed
		{
			D_H_DETAILS_DRAW_PROPERTY("Speed");

			valueChanged |= ImGui::DragFloat("##Speed", &mSpeed);
		}

		// Speed
		{
			D_H_DETAILS_DRAW_PROPERTY("Mouse Speed");

			valueChanged |= ImGui::DragFloat("##MouseSpeed", &mMouseSpeed);
		}

		// Jum Speed
		{
			D_H_DETAILS_DRAW_PROPERTY("Jump Speed");

			valueChanged |= ImGui::DragFloat("##JumpSpeed", &mJumpSpeed);
		}

		// Mouse Keyboard
		{
			D_H_DETAILS_DRAW_PROPERTY("Use Mouse & Keyboard");

			valueChanged |= ImGui::Checkbox("##UseMouseKeyboard", &mUseMouseKeyboard);
		}

		// Mouse Keyboard
		{
			D_H_DETAILS_DRAW_PROPERTY("Gamepad Look Sensitivity");

			valueChanged |= ImGui::DragFloat("##GamepadLookSens", &mGamepadLookSens);
		}

		// Upper Body
		{
			D_H_DETAILS_DRAW_PROPERTY("Upper Body");
			D_H_GAMEOBJECT_SELECTION_DRAW_SIMPLE(mUpperBody);
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

}
