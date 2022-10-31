#include "pch.hpp"
#include "AnimationComponent.hpp"

#include <imgui.h>

namespace Darius::Animation
{
	D_H_COMP_DEF(AnimationComponent);

	AnimationComponent::AnimationComponent() :
		ComponentBase()
	{ }

	AnimationComponent::AnimationComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid)
	{ }

	void AnimationComponent::SetAnimation(ResourceHandle handle)
	{
		mChangeSignal();
		_SetAnimation(handle);
	}

	void AnimationComponent::_SetAnimation(ResourceHandle handle)
	{
		mAnimationResource = D_RESOURCE::GetResource<AnimationResource>(handle, *GetGameObject());
	}

#ifdef _DEBUG
	bool AnimationComponent::DrawDetails(float params[])
	{
		bool changeValue = false;

		if (ImGui::BeginTable("##componentLayout", 2, ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 100.f);
			ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Animation Clip");
			ImGui::TableSetColumnIndex(1);
			// Mesh selection
			D_H_RESOURCE_SELECTION_DRAW(AnimationResource, mAnimationResource, "Select Animation", SetAnimation);

			ImGui::EndTable();
		}

		return changeValue;
	}
#endif

}

