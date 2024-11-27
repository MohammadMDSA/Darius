#include <pch.hpp>
#include "SpaceInvadersBullet.hpp"

#include <ResourceManager/Resource.hpp>

#ifdef _D_EDITOR
#include <Scene/Utils/GameObjectDragDropPayload.hpp>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "SpaceInvadersBullet.sgenerated.hpp"

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(SpaceInvadersBullet);

	SpaceInvadersBullet::SpaceInvadersBullet() :
		Super()
	{ }

	SpaceInvadersBullet::SpaceInvadersBullet(D_CORE::Uuid const& uuid) :
		Super(uuid)
	{ }

	void SpaceInvadersBullet::Start()
	{

	}

	void SpaceInvadersBullet::Update(float deltaTime)
	{
		auto trans = GetTransform();

		auto movement = trans->GetForward() * mSpeed * deltaTime;
		trans->SetPosition(trans->GetPosition() + movement);
	}

	void SpaceInvadersBullet::SetShooterPlayer(int id)
	{
		if (id < 0)
			return;

		mShooterPlayerId = id;
	}

#ifdef _D_EDITOR
	bool SpaceInvadersBullet::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();


		// sample field
		D_H_DETAILS_DRAW_PROPERTY("field");

		float val;
		valueChanged |= ImGui::InputFloat("##val", &val);

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

}
