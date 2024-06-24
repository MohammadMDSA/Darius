#include <pch.hpp>
#include "Gun.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Physics/Components/RigidbodyComponent.hpp>
#include <ResourceManager/Resource.hpp>

#ifdef _D_EDITOR
#include <Scene/Utils/GameObjectDragDropPayload.hpp>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "Gun.sgenerated.hpp"

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(Gun);

	Gun::Gun() :
		Super()
	{ }

	Gun::Gun(D_CORE::Uuid const& uuid) :
		Super(uuid)
	{ }

	void Gun::Start()
	{
		mFlash->SetActive(false);
	}

	void Gun::Update(float deltaTime)
	{
		if(mFlash->IsActive())
		{
			auto time = D_TIME::GetTotalTime();
			if(time >= mFlashTime + mFlashStartTime)
				mFlash->SetActive(false);
		}
	}

	void Gun::Fire(D_MATH::Vector3 const& direction)
	{
		mFlashStartTime = D_TIME::GetTotalTime();
		if(!mBulletPrefab.IsValid())
			return;
		auto bullet = D_WORLD::InstantiateGameObject(mBulletPrefab.Get());
		auto transform = bullet->GetTransform();
		transform->SetPosition(mBulletSpawnTransform->GetPosition());
		transform->SetRotation(D_MATH::Quaternion::FromForwardAndAngle(direction));

		if(auto rigid = bullet->GetComponent<D_PHYSICS::RigidbodyComponent>())
		{
			rigid->SetLinearVelocity(direction.Normal() * mBulletSpeed);
		}

		mFlash->SetActive(true);
	}

#ifdef _D_EDITOR
	bool Gun::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();


		// Bullet Prefab
		{
			D_H_DETAILS_DRAW_PROPERTY("Bullet");
			D_H_GAMEOBJECT_SELECTION_DRAW_SIMPLE(mBulletPrefab);
		}

		// Flash GameObject
		{
			D_H_DETAILS_DRAW_PROPERTY("Muzzle Flash");
			D_H_GAMEOBJECT_SELECTION_DRAW_SIMPLE(mFlash);
		}

		// Flash Time
		{
			D_H_DETAILS_DRAW_PROPERTY("Flash Time");
			valueChanged |= ImGui::DragFloat("##FlashTime", &mFlashTime);
		}

		// Bullet Speed
		{
			D_H_DETAILS_DRAW_PROPERTY("Bullet Speed");
			valueChanged |= ImGui::DragFloat("##BulletSpeed", &mBulletSpeed);
		}

		// Spawn Transform
		{
			D_H_DETAILS_DRAW_PROPERTY("Bullet Spawn Point");

			D_H_COMPONENT_SELECTION_DRAW_SIMPLE(D_MATH::TransformComponent, mBulletSpawnTransform);
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

}
