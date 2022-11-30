#include "pch.hpp"
#include "PhysicsActor.hpp"

#include "PhysicsManager.hpp"
#include "PhysicsScene.hpp"

using namespace physx;

namespace Darius::Physics
{
	PhysicsActor::PhysicsActor(D_SCENE::GameObject* gameObject, PhysicsActorType type) :
		mActorType(type),
		mPxActor(nullptr),
		mGameObject(gameObject),
		mColliderCount(0)
	{
	}

	PhysicsActor::~PhysicsActor()
	{
		if (!mPxActor)
			return;
		auto scene = D_PHYSICS::GetScene();
		scene->removeActor(*mPxActor);
		mPxActor = nullptr;
	}

	void PhysicsActor::InitializeActor()
	{
		D_ASSERT(mGameObject);
		if (mPxActor)
			return;

		auto scene = D_PHYSICS::GetScene();
		auto physics = D_PHYSICS::GetCore();

		// Create actor initial transform
		auto mainTransform = mGameObject->GetTransform();
		auto transform = D_PHYSICS::GetTransform(mainTransform);

		// Create proper actor if doesn't exist
		if (IsStatic())
		{
			mPxActor = physics->createRigidStatic(transform);
		}
		else
		{
			mPxActor = physics->createRigidDynamic(transform);
		}

		scene->addActor(*mPxActor);

	}
}
