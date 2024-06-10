#include <pch.hpp>
#include "LimitedLifeTimeComponent.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <ResourceManager/Resource.hpp>

#ifdef _D_EDITOR
#include <Scene/Utils/GameObjectDragDropPayload.hpp>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "LimitedLifeTimeComponent.sgenerated.hpp"

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(LimitedLifeTimeComponent);

	LimitedLifeTimeComponent::LimitedLifeTimeComponent() :
		Super()
	{ }

	LimitedLifeTimeComponent::LimitedLifeTimeComponent(D_CORE::Uuid const& uuid) :
		Super(uuid)
	{ }

	void LimitedLifeTimeComponent::Start()
	{
		mStartTime = D_TIME::GetTotalTime();
	}

	void LimitedLifeTimeComponent::Update(float deltaTime)
	{
		if(mStartTime + mTimeToLive <= D_TIME::GetTotalTime())
			D_WORLD::DeleteGameObject(GetGameObject());
	}

#ifdef _D_EDITOR
	bool LimitedLifeTimeComponent::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();


		// sample field
		D_H_DETAILS_DRAW_PROPERTY("Time to Live");

		valueChanged |= ImGui::DragFloat("##TimeToLive", &mTimeToLive, 1.f, 0.f);

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

}
