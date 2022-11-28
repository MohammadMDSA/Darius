#include "Physics/pch.hpp"
#include "RigidBodyComponent.hpp"

#include "Physics/PhysicsManager.hpp"
#include "Physics/PhysicsScene.hpp"

namespace Darius::Physics
{
	D_H_COMP_DEF(RigidbodyComponent);

	D_H_COMP_DEFAULT_CONSTRUCTOR_DEF(RigidbodyComponent);

	void RigidbodyComponent::Start()
	{
		mActor = D_PHYSICS::PhysicsScene::AddDynamicActor(GetGameObject());
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

}
