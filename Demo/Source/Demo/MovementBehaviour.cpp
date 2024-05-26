#include <pch.hpp>
#include "MovementBehaviour.hpp"

#include <Core/TimeManager/TimeManager.hpp>


#include <imgui.h>

#include "MovementBehaviour.sgenerated.hpp"

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(MovementBehaviour);

	MovementBehaviour::MovementBehaviour() :
		D_ECS_COMP::BehaviourComponent(),
		mAxis(D_MATH::Vector3::Up),
		mRange(5.f)
	{ }

	MovementBehaviour::MovementBehaviour(D_CORE::Uuid const& uuid) :
		D_ECS_COMP::BehaviourComponent(uuid),
		mAxis(D_MATH::Vector3::Up),
		mRange(5.f)
	{ }

	void MovementBehaviour::Start()
	{

	}

	void MovementBehaviour::Update(float deltaTime)
	{
		auto time = D_TIME::GetTotalTime();
		auto axis = mAxis.Normal();

		auto trans = GetTransform();
		auto pos = axis * D_MATH::Cos(time * 2) * mRange;
		trans->SetPosition(pos);
		if (mRotate)
			trans->SetRotation(D_MATH::Quaternion(D_MATH::Vector3::Up, time));
	}

#ifdef _D_EDITOR
	bool MovementBehaviour::DrawDetails(float params[])
	{
		bool changed = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();


		// Rotate
		D_H_DETAILS_DRAW_PROPERTY("Rotate");
		changed |= ImGui::Checkbox("##Rotate", &mRotate);

		// Axis
		D_H_DETAILS_DRAW_PROPERTY("Axis");
		changed |= D_MATH::DrawDetails(mAxis, D_MATH::Vector3::Up);

		// Range
		D_H_DETAILS_DRAW_PROPERTY("Range");
		changed |= ImGui::DragFloat("##Range", &mRange, 0.1f, 0.f);

		D_H_DETAILS_DRAW_END_TABLE();

		return changed;
	}
#endif

}
