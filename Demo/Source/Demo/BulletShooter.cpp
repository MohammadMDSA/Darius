#include <pch.hpp>
#include "BulletShooter.hpp"

#include <Core/Input.hpp>
#include <Physics/Components/RigidbodyComponent.hpp>
#include <ResourceManager/Resource.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

#include "Gun.hpp"

#ifdef _D_EDITOR
#include <Scene/Utils/GameObjectDragDropPayload.hpp>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "BulletShooter.sgenerated.hpp"

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(BulletShooter);

	BulletShooter::BulletShooter() :
		Super()
	{ }

	BulletShooter::BulletShooter(D_CORE::Uuid const& uuid) :
		Super(uuid)
	{ }

	void BulletShooter::Update(float deltaTime)
	{
		if(D_INPUT::IsFirstPressed(D_INPUT::DigitalInput::Mouse0) || D_INPUT::IsFirstPressed(D_INPUT::DigitalInput::RShoulder))
		{
			if(!mGun.IsValid())
				return;
			
			mGun->Fire(mRefAim->GetForward());
		}
	}

#ifdef _D_EDITOR
	bool BulletShooter::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();


		// RefAim
		{
			D_H_DETAILS_DRAW_PROPERTY("Aim Ref Trans");
			D_H_COMPONENT_SELECTION_DRAW_SIMPLE(D_MATH::TransformComponent, mRefAim);
		}

		// Gun
		{
			D_H_DETAILS_DRAW_PROPERTY("Gun");
			D_H_COMPONENT_SELECTION_DRAW_SIMPLE(Gun, mGun);
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

}
