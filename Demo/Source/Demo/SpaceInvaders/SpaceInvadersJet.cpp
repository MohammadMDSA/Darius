#include <pch.hpp>
#include "SpaceInvadersJet.hpp"

#include "SpaceInvadersBullet.hpp"

#include <Core/Input.hpp>
#include <Physics/Components/BoxColliderComponent.hpp>
#include <ResourceManager/Resource.hpp>

#ifdef _D_EDITOR
#include <Scene/Utils/GameObjectDragDropPayload.hpp>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "SpaceInvadersJet.sgenerated.hpp"

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(SpaceInvadersJet);

	SpaceInvadersJet::SpaceInvadersJet() :
		Super()
	{ }

	SpaceInvadersJet::SpaceInvadersJet(D_CORE::Uuid const& uuid) :
		Super(uuid)
	{ }

	void SpaceInvadersJet::Start()
	{
		auto collider = GetGameObject()->GetComponent<D_PHYSICS::BoxColliderComponent>();

		mTriggerSignal = collider->OnTriggerEnter.ConnectComponent(this, &ThisClass::OnTrigger);
	}

	void SpaceInvadersJet::OnDestroy()
	{
		mTriggerSignal.disconnect();
	}

	void SpaceInvadersJet::Update(float deltaTime)
	{
		if (mIsPlayer)
			PlayerControl(deltaTime);
		else
			AiControl(deltaTime);
	}

	void SpaceInvadersJet::OnTrigger(Darius::Physics::ColliderComponent* thisCollider, D_SCENE::GameObject* otherGameObject)
	{
		auto bullet = otherGameObject->GetComponent<SpaceInvadersBullet>();
		if (!bullet)
			return;

		if (bullet->GetShooterPlayer() == GetPlayerId())
			return;

		D_WORLD::DeleteGameObject(otherGameObject);
		D_WORLD::DeleteGameObject(GetGameObject());
	}


	void SpaceInvadersJet::PlayerControl(float dt)
	{
		using namespace D_INPUT;
		float inputHor = 0.f;
		if (D_INPUT::IsPressed(DigitalInput::KeyLeft))
			inputHor += -1.f;
		if (D_INPUT::IsPressed(DigitalInput::KeyRight))
			inputHor += 1.f;

		float inputVert = 0.f;
		if (D_INPUT::IsPressed(DigitalInput::KeyUp))
			inputVert += 1.f;
		if (D_INPUT::IsPressed(DigitalInput::KeyDown))
			inputVert += -1.f;

		auto trans = GetTransform();

		D_MATH::Vector3 wsDir = trans->GetForward() * inputVert + trans->GetRight() * inputHor;

		Move(wsDir.Normal(), dt);

		if (D_INPUT::IsFirstPressed(DigitalInput::KeySpace))
			Fire();
	}

	void SpaceInvadersJet::AiControl(float dt)
	{
		// Movement
		mAiCurrentDirectionMovementTime += dt;
		if (mAiCurrentDirectionMovementTime >= mAiDirectionMovementTime)
		{
			mAiCurrentDirectionMovementTime = 0.f;
			mAiMovingRight = !mAiMovingRight;
		}

		auto trans = GetTransform();

		D_MATH::Vector3 movement = mAiMovingRight ? trans->GetRight() : trans->GetLeft();
		Move(movement.Normal(), dt);


		// Firing
		mAiLastFireTime += dt;
		if (mAiLastFireTime >= mAiFireCooldown)
		{
			mAiLastFireTime = 0.f;
			Fire();
		}
	}

	void SpaceInvadersJet::Move(D_MATH::Vector3 const& normalizedDir, float dt)
	{

		D_MATH::Vector3 dir = normalizedDir;
		if (dir.Length() > 1)
			dir.Normalize();

		D_MATH::Vector3 movement = dir * mSpeed * dt;
		auto trans = GetTransform();
		trans->SetPosition(trans->GetPosition() + movement);

	}

	void SpaceInvadersJet::Fire()
	{
		if (!mBulletPrefab.IsValid())
			return;

		if (!mBulletSpawnTransform.IsValid())
			return;

		auto bullet = D_WORLD::InstantiateGameObject(mBulletPrefab.Get());
		auto bulletTrans = bullet->GetTransform();
		bulletTrans->SetPosition(mBulletSpawnTransform->GetPosition());
		bulletTrans->SetRotation(mBulletSpawnTransform->GetRotation());

		auto bulletComp = bullet->GetComponent<SpaceInvadersBullet>();
		D_VERIFY(bulletComp);
		bulletComp->SetShooterPlayer(GetPlayerId());
	}

#ifdef _D_EDITOR
	bool SpaceInvadersJet::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Is Player
		{
			D_H_DETAILS_DRAW_PROPERTY("Player");
			valueChanged |= ImGui::Checkbox("##IsPlayer", &mIsPlayer);
		}

		// Bullet Pref
		{
			D_H_DETAILS_DRAW_PROPERTY("Bullet Prefab");
			D_H_GAMEOBJECT_SELECTION_DRAW_SIMPLE(mBulletPrefab);
		}

		// Bullet Spawn Trans
		{
			D_H_DETAILS_DRAW_PROPERTY("Bullet Spawn Trans");
			D_H_COMPONENT_SELECTION_DRAW_SIMPLE(D_MATH::TransformComponent, mBulletSpawnTransform);
		}

		// Speed
		{
			D_H_DETAILS_DRAW_PROPERTY("Speed");
			valueChanged |= ImGui::DragFloat("##Speed", &mSpeed, 0.1f, 0.1f);
		}

		// Ai Direction Movement Time
		{
			D_H_DETAILS_DRAW_PROPERTY("Ai Direction Movement Time");
			valueChanged |= ImGui::DragFloat("##Ai Direction Movement Time", &mAiDirectionMovementTime, 0.1f, 0.5f);
		}

		// Ai Fire Cooldown
		{
			D_H_DETAILS_DRAW_PROPERTY("Ai Fire Cooldown");
			valueChanged |= ImGui::DragFloat("##Ai Fire Cooldown", &mAiFireCooldown, 0.1f, 0.1f);
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

}
