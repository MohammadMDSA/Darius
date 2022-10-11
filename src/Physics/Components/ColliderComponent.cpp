#include "Physics/pch.hpp"
#include "ColliderComponent.hpp"

#include <imgui.h>

using namespace physx;

namespace Darius::Physics
{
	D_H_COMP_DEF(ColliderComponent);

	D_H_COMP_DEFAULT_CONSTRUCTOR_DEF(ColliderComponent);

	bool ColliderComponent::DrawDetails(float params[])
	{
		if (ImGui::Checkbox("Is static", &mIsStatic))
		{
			InitActor();
			return true;
		}
		return false;

	}

	void ColliderComponent::Start()
	{
		InitActor();
	}

	void ColliderComponent::InitActor()
	{
		auto pxScene = D_PHYSICS::GetScene();
		auto physics = D_PHYSICS::GetCore();

		// Remove actor if the type is incorrect
		if (mActor)
		{
			auto type = mActor->getType();
			if ((type == PxActorType::eRIGID_STATIC && IsDynamic()) ||
				(type == PxActorType::eRIGID_DYNAMIC && !IsDynamic()))
			{
				pxScene->removeActor(*mActor);
				mActor->release();
				mActor = nullptr;
			}
		}

		// Create actor initial transform
		auto mainTransform = GetTransform();
		auto transform = D_PHYSICS::GetTransform(mainTransform);

		// Create proper actor if doesn't exist
		if (!mActor)
		{
			if (IsDynamic())
			{
				mActor = physics->createRigidDynamic(transform);
			}
			else
			{
				mActor = physics->createRigidStatic(transform);
			}
		}

		if (mShape)
		{
			mShape = nullptr;
		}

		auto geom = GetPhysicsGeometry();
		mShape = physx::PxRigidActorExt::createExclusiveShape(*mActor, *geom, *D_PHYSICS::GetDefaultMaterial());

		pxScene->addActor(*mActor);

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

		// Updating scale, pos, rot
		bool geomChanged;
		auto geom = UpdateAndGetPhysicsGeometry(geomChanged);

		if (geomChanged)
		{
			mShape->setGeometry(*geom);
		}

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
