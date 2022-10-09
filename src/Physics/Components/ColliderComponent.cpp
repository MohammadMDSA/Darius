#include "Physics/pch.hpp"
#include "ColliderComponent.hpp"

#include <imgui.h>

namespace Darius::Physics
{
	D_H_COMP_DEF(ColliderComponent);

	D_H_COMP_DEFAULT_CONSTRUCTOR_DEF(ColliderComponent);

	bool ColliderComponent::DrawDetails(float params[])
	{
		return ImGui::Checkbox("Is static", &mIsStatic);

	}

	void ColliderComponent::Awake()
	{
		auto physics = D_PHYSICS::GetCore();

		auto material = D_PHYSICS::GetDefaultMaterial();

		auto mainTransform = GetTransform();
		auto transform = D_PHYSICS::GetTransform(mainTransform);

		auto geom = physx::PxBoxGeometry(mainTransform.Scale.GetX() / 2.f, mainTransform.Scale.GetY() / 2.f, mainTransform.Scale.GetZ() / 2.f);


		if (mIsStatic)
		{
			mActor = physics->createRigidStatic(transform);
		}
		else
		{
			mActor = physics->createRigidDynamic(transform);
		}

		mShape = physx::PxRigidActorExt::createExclusiveShape(*mActor, geom, *material);

		D_PHYSICS::GetScene()->addActor(*mActor);
	}

	void ColliderComponent::Update(float dt)
	{
		auto preTrans = GetTransform();
		D_MATH::Transform physicsTrans = D_PHYSICS::GetTransform(physx::PxShapeExt::getGlobalPose(*mShape, *mActor));
		preTrans.Translation = physicsTrans.Translation;
		preTrans.Rotation = physicsTrans.Rotation;
		SetTransform(preTrans);
	}

	void ColliderComponent::PreUpdate()
	{
		if (!mIsStatic)
			mActor->setGlobalPose(D_PHYSICS::GetTransform(GetTransform()));
	}

	void ColliderComponent::Serialize(D_SERIALIZATION::Json& json) const
	{
		json["Static"] = mIsStatic;
	}

	void ColliderComponent::Deserialize(D_SERIALIZATION::Json const& json)
	{
		mIsStatic = json["Static"];
	}

	void ColliderComponent::OnDestroy()
	{
		auto phScene = D_PHYSICS::GetScene();
		phScene->removeActor(*mActor);
	}
}
