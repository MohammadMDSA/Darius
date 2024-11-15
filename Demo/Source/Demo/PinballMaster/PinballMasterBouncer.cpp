#include <pch.hpp>
#include "PinballMasterBouncer.hpp"

#include <Physics/Components/CapsuleColliderComponent.hpp>
#include <Physics/HitResult.hpp>
#include <ResourceManager/Resource.hpp>

#ifdef _D_EDITOR
#include <Scene/Utils/GameObjectDragDropPayload.hpp>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "PinballMasterBouncer.sgenerated.hpp"

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(PinballMasterBouncer);

	PinballMasterBouncer::PinballMasterBouncer() :
		Super()
	{ }

	PinballMasterBouncer::PinballMasterBouncer(D_CORE::Uuid const& uuid) :
		Super(uuid)
	{ }

	void PinballMasterBouncer::Start()
	{
		auto collider = GetGameObject()->GetComponent<D_PHYSICS::CapsuleColliderComponent>();
		if (!collider)
			return;

		mColliderConnection = collider->OnColliderContactEnter.ConnectComponent(this, &ThisClass::OnColliderEnter);

		mForceTarget = nullptr;
	}

	void PinballMasterBouncer::Update(float deltaTime)
	{
		if (!mForceTarget.IsValid())
		{
			mForceTarget = nullptr;
			return;
		}

		mForceTarget->AddForce(mForceDir * mForceAmount, D_PHYSICS::ForceMode::Impulse);
		mForceTarget = nullptr;
	}

	void PinballMasterBouncer::OnDestroy()
	{
		mColliderConnection.disconnect();
	}

	void PinballMasterBouncer::OnColliderEnter(Darius::Physics::ColliderComponent* thisCollider, Darius::Physics::ColliderComponent* otherCollider, D_SCENE::GameObject* otherGameObject, Darius::Physics::HitResult const& Hit)
	{
		if (Hit.Contacts.size() <= 0)
			return;

		auto otherRigid = otherGameObject->GetComponent<D_PHYSICS::RigidbodyComponent>();
		if (!otherRigid)
			return;

		auto const& contact = Hit.Contacts[0];
		auto normal = contact.Normal;

		mForceDir = normal.Normal();
		mForceTarget = otherRigid;
	}

#ifdef _D_EDITOR
	bool PinballMasterBouncer::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();


		// sample field
		D_H_DETAILS_DRAW_PROPERTY("Force Amount");

		valueChanged |= ImGui::DragFloat("##ForceAmount", &mForceAmount, 0.1f);

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

}
