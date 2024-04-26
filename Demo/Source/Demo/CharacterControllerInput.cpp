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
		mJumpSpeed(1.f)
	{ }

	CharacterControllerInput::CharacterControllerInput(D_CORE::Uuid const& uuid) :
		D_ECS_COMP::BehaviourComponent(uuid),
		mSpeed(10.f),
		mMouseSpeed(0.001f),
		mJumpSpeed(1.f)
	{ }

	void CharacterControllerInput::Start()
	{
		mController = GetGameObject()->GetComponent<D_PHYSICS::CharacterControllerComponent>();
	}
	
	void CharacterControllerInput::OnDestroy()
	{
	}

	void CharacterControllerInput::Update(float deltaTime)
	{
		
		Vector3 movement = Vector3::Zero;
		if(D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyW))
			movement += Vector3::Forward;
		if(D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyS))
			movement += Vector3::Backward;
		if(D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyA))
			movement += Vector3::Left;
		if(D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyD))
			movement += Vector3::Right;

		movement.Normalize();

		float horizontal = D_INPUT::GetAnalogInput(D_INPUT::AnalogInput::MouseX);
		auto rot = Quaternion(Vector3::Up, D_MATH::Rad2Deg(-horizontal * mMouseSpeed));

		auto trans = GetTransform();

		auto newRot = trans->GetRotation() * rot;
		trans->SetRotation(newRot);

		if(mController.IsValid())
			mController->Move((newRot * movement) * mSpeed * deltaTime);

		if(D_INPUT::IsFirstPressed(D_INPUT::DigitalInput::KeySpace))
			mController->Jump(mJumpSpeed);
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

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

}
