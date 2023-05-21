#include <pch.hpp>
#include "Targeter.hpp"

#include <ResourceManager/Resource.hpp>

#ifdef _D_EDITOR
#include <Scene/Utils/GameObjectDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "Targeter.sgenerated.hpp"

using namespace D_SCENE;

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(Targeter);

	Targeter::Targeter() :
		D_ECS_COMP::BehaviourComponent(),
		mTargetObject(GetAsCountedOwner())
	{ }

	Targeter::Targeter(D_CORE::Uuid uuid) :
		D_ECS_COMP::BehaviourComponent(uuid),
		mTargetObject(GetAsCountedOwner())
	{ }

	void Targeter::Start()
	{

	}

	void Targeter::Update(float deltaTime)
	{
		auto trans = GetTransform();
		trans.Rotation = D_MATH::LookAt(trans.Translation, mTargetObject->GetTransform().Translation);
		SetTransform(trans);
	}

#ifdef _D_EDITOR
	bool Targeter::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();


		// sample field
		D_H_DETAILS_DRAW_PROPERTY("field");

		float val;
		valueChanged |= ImGui::InputFloat("##val", &val);


		{
			D_H_DETAILS_DRAW_PROPERTY("Target");

			bool isMissing;
			std::string displayText;
			if (mTargetObject.IsValid(isMissing))
				displayText = mTargetObject->GetName();
			else if (isMissing)
				displayText = "Missing (Game Object)";
			else
				displayText = "<None>";

			auto availableSpalce = ImGui::GetContentRegionAvail();
			auto selectionWidth = 20.f;

			ImGui::Button(displayText.c_str(), ImVec2(availableSpalce.x - 2 * selectionWidth - 10.f, 0));

			// DragDrop
			if (ImGui::BeginDragDropTarget())
			{
				ImGuiPayload const* imPayload = ImGui::GetDragDropPayload();
				auto payload = reinterpret_cast<Darius::Utils::BaseDragDropPayloadContent const*>(imPayload->Data);
				if (payload && payload->IsCompatible(D_UTILS::BaseDragDropPayloadContent::Type::GameObject, "GameObject"))
				{
					if (ImGuiPayload const* acceptedPayload = ImGui::AcceptDragDropPayload(D_PAYLOAD_TYPE_GAMEOBJECT))
					{
						auto gameObject = reinterpret_cast<D_SCENE::GameObjectDragDropPayloadContent const*>(acceptedPayload->Data)->GameObject;
						SetTarget(gameObject);
					}
				}
				ImGui::EndDragDropTarget();
			}
		}


		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

	void Targeter::SetTarget(GameObject* go)
	{
		mTargetObject = GameObjectRef(go);
	}

}
