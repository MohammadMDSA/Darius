#include <pch.hpp>
#include "ComponentReferencer.hpp"

#include <ResourceManager/Resource.hpp>

#ifdef _D_EDITOR
#include <Scene/Utils/GameObjectDragDropPayload.hpp>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "ComponentReferencer.sgenerated.hpp"

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(ComponentReferencer);

	ComponentReferencer::ComponentReferencer() :
		D_ECS_COMP::BehaviourComponent()
	{ }

	ComponentReferencer::ComponentReferencer(D_CORE::Uuid const& uuid) :
		D_ECS_COMP::BehaviourComponent(uuid)
	{ }

	void ComponentReferencer::Start()
	{
		
	}

	void ComponentReferencer::Update(float deltaTime)
	{
		if (!mOtherTrans.IsValid())
			return;

		auto trans = GetTransform();
		trans->SetRotation(D_MATH::LookAt(trans->GetPosition(), mOtherTrans->GetPosition()));
	}

	void ComponentReferencer::SetOtherTrans(D_MATH::TransformComponent* other)
	{
		if (mOtherTrans == other)
			return;

		mOtherTrans = other;

		mChangeSignal(this);
	}

#ifdef _D_EDITOR
	bool ComponentReferencer::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Other Comp Ref
		{
			D_H_DETAILS_DRAW_PROPERTY("Other Trans");

			ImGui::BeginGroup();

			bool isMissing = mOtherTrans.IsMissing();
			std::string displayText;
			if (mOtherTrans.IsValid())
				displayText = mOtherTrans.GetGameObject()->GetName();
			else if (isMissing)
				displayText = "Missing Game Object";
			else
				displayText = "<None>";
			auto availableSpace = ImGui::GetContentRegionAvail();
			auto selectionWidth = 20.f;

			ImGui::Button(displayText.c_str(), ImVec2(availableSpace.x - 2 * selectionWidth - 10.f, 0));
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				SetOtherTrans(nullptr);

			{
				if (ImGui::BeginDragDropTarget())
				{
					ImGuiPayload const* imPayload = ImGui::GetDragDropPayload();
					auto payload = reinterpret_cast<Darius::Utils::BaseDragDropPayloadContent const*>(imPayload->Data);
					if (payload && payload->PayloadType != D_UTILS::BaseDragDropPayloadContent::Type::Invalid && payload->IsCompatible(D_UTILS::BaseDragDropPayloadContent::Type::Component, D_MATH::TransformComponent::ClassName()))
					{
						if (ImGuiPayload const* acceptedPayload = ImGui::AcceptDragDropPayload(D_PAYLOAD_TYPE_GAMEOBJECT))
						{
							auto gameObject = reinterpret_cast<D_SCENE::GameObjectDragDropPayloadContent const*>(acceptedPayload->Data)->GameObjectRef;
							SetOtherTrans(gameObject->GetComponent<D_MATH::TransformComponent>());
						}
					}
					ImGui::EndDragDropTarget();
				}
			}

			ImGui::EndGroup();
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

}
