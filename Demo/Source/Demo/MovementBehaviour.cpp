#include <pch.hpp>
#include "MovementBehaviour.hpp"

#include <Core/TimeManager/TimeManager.hpp>


#include <imgui.h>

#include "MovementBehaviour.sgenerated.hpp"

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(MovementBehaviour);

	MovementBehaviour::MovementBehaviour() :
		D_ECS_COMP::BehaviourComponent()
	{ }

	MovementBehaviour::MovementBehaviour(D_CORE::Uuid const& uuid) :
		D_ECS_COMP::BehaviourComponent(uuid)
	{ }

	void MovementBehaviour::Start()
	{

	}

	void MovementBehaviour::Update(float deltaTime)
	{
		auto time = D_TIME::GetTotalTime();

		auto trans = GetTransform();
		trans->SetPosition(D_MATH::Vector3(0.f, D_MATH::Cos(time * 2) * 5, 0.f));
		if (mRotate)
			trans->SetRotation(D_MATH::Quaternion(D_MATH::Vector3::Up, time));
	}

#ifdef _D_EDITOR
	bool MovementBehaviour::DrawDetails(float params[])
	{
		bool changed = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();


		// sample field
		D_H_DETAILS_DRAW_PROPERTY("Rotate");

		changed |= ImGui::Checkbox("##Rotate", &mRotate);

		D_H_DETAILS_DRAW_END_TABLE();

		return changed;
	}
#endif

}
