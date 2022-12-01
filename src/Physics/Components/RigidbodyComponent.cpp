#include "Physics/pch.hpp"
#include "RigidbodyComponent.hpp"

#include "Physics/PhysicsManager.hpp"
#include "Physics/PhysicsScene.hpp"

#include <imgui.h>

using namespace physx;

namespace Darius::Physics
{
	D_H_COMP_DEF(RigidbodyComponent);

	D_H_COMP_DEFAULT_CONSTRUCTOR_DEF(RigidbodyComponent);

	void RigidbodyComponent::Start()
	{
		mActor = D_PHYSICS::PhysicsScene::AddDynamicActor(GetGameObject(), false);
	}
	
	void RigidbodyComponent::OnDestroy()
	{
		D_PHYSICS::PhysicsScene::RemoveDynamicActor(GetGameObject());
	}

	void RigidbodyComponent::Update(float)
	{
		auto preTrans = GetTransform();
		D_MATH::Transform physicsTrans = D_PHYSICS::GetTransform(mActor->getGlobalPose());
		preTrans.Translation = physicsTrans.Translation;
		preTrans.Rotation = physicsTrans.Rotation;
		SetTransform(preTrans);
	}

	void RigidbodyComponent::PreUpdate()
	{
		mActor->setGlobalPose(D_PHYSICS::GetTransform(GetTransform()));
	}

	void RigidbodyComponent::SetKinematic(bool value)
	{
		mActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, value);
	}

	bool RigidbodyComponent::IsKinematic() const
	{
		return mActor->getRigidBodyFlags().isSet(PxRigidBodyFlag::eKINEMATIC);
	}

#ifdef _D_EDITOR
	bool RigidbodyComponent::DrawDetails(float[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Kinematic property
		{
			D_H_DETAILS_DRAW_PROPERTY("Kinematic");
			bool kinematic = IsKinematic();
			if (ImGui::Checkbox("##kinematic", &kinematic))
			{
				SetKinematic(kinematic);
				valueChanged = true;
			}
			
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

}
