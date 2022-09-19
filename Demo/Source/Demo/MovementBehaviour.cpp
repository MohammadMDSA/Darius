#include <pch.hpp>
#include "MovementBehaviour.hpp"

#include <Core/TimeManager/TimeManager.hpp>


#include <imgui.h>

using namespace D_ECS_COMP;

namespace Demo
{
	D_H_COMP_DEF(MovementBehaviour);

	MovementBehaviour::MovementBehaviour() :
		BehaviourComponent()
	{ }

	MovementBehaviour::MovementBehaviour(D_CORE::Uuid uuid) :
		BehaviourComponent(uuid)
	{ }

	void MovementBehaviour::Start()
	{

	}

	void MovementBehaviour::Update(float deltaTime)
	{
		auto time = D_TIME::GetTotalTime();

		auto trans = GetTransform();
		trans.Translation = D_MATH::Vector3(0.f, D_MATH::Cos(time) * 5, 0.f);
		SetTransform(trans);
	}

#ifdef _DEBUG
	bool MovementBehaviour::DrawDetails(float params[])
	{
		bool changed = false;

		if (ImGui::BeginTable("##componentLayout", 2, ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 100.f);
			ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);


			// sample field
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("field");

			ImGui::TableSetColumnIndex(1);
			float val;
			changed |= ImGui::InputFloat("##val", &val);

			// Ending table
			ImGui::EndTable();
		}

		return false;
	}
#endif

}
