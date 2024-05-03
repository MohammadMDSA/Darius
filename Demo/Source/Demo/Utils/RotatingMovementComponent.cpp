#include <pch.hpp>
#include "RotatingMovementComponent.hpp"

#include <ResourceManager/Resource.hpp>

#ifdef _D_EDITOR
#include <Scene/Utils/GameObjectDragDropPayload.hpp>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "RotatingMovementComponent.sgenerated.hpp"

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(RotatingMovementComponent);

	RotatingMovementComponent::RotatingMovementComponent() :
		D_ECS_COMP::BehaviourComponent(),
		mSpeed(90.f),
		mAxis(D_MATH::Vector3::Up),
		mLocal(true)
	{ }

	RotatingMovementComponent::RotatingMovementComponent(D_CORE::Uuid const& uuid) :
		D_ECS_COMP::BehaviourComponent(uuid),
		mSpeed(90.f),
		mAxis(D_MATH::Vector3::Up),
		mLocal(true)
	{ }

	void RotatingMovementComponent::Update(float deltaTime)
	{
		using namespace D_MATH;

		Quaternion deltaRot(mAxis, D_MATH::Deg2Rad(mSpeed * deltaTime));
		
		auto trans = GetTransform();

		if(IsLocal())
		{
			Quaternion currentRot = trans->GetLocalRotation();
			trans->SetLocalRotation(currentRot * deltaRot);
		}
		else
		{
			Quaternion currentRot = trans->GetRotation();
			trans->SetRotation(currentRot * deltaRot);
		}
	}

#ifdef _D_EDITOR
	bool RotatingMovementComponent::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Speed
		{
			D_H_DETAILS_DRAW_PROPERTY("Speed");
			auto value = GetSpeed();
			if(ImGui::DragFloat("##Speed", &value))
			{
				SetSpeed(value);
				valueChanged = true;
			}
		}

		// Axis
		{
			D_H_DETAILS_DRAW_PROPERTY("Axis");
			auto value = GetAxis();
			if(D_MATH::DrawDetails(value, D_MATH::Vector3::Up))
			{
				SetAxis(value);
				valueChanged = true;
			}
		}

		// Local
		{
			D_H_DETAILS_DRAW_PROPERTY("Local");
			auto value = IsLocal();
			if(ImGui::Checkbox("##Local", &value))
			{
				SetLocal(value);
				valueChanged = true;
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

	void RotatingMovementComponent::SetLocal(bool local)
	{
		if(mLocal == local)
			return;

		mLocal = local;

		mChangeSignal(this);
	}

	void RotatingMovementComponent::SetSpeed(float speed)
	{
		if(mSpeed == speed)
			return;

		mSpeed = speed;

		mChangeSignal(this);
	}

	void RotatingMovementComponent::SetAxis(D_MATH::Vector3 const& axis)
	{
		if(axis.IsZero())
			return;

		if(mAxis.Equals(axis))
			return;

		mAxis = axis;

		mChangeSignal(this);
	}

}
