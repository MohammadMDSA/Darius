#include "pch.hpp"
#include "PhysicsActor.hpp"

#include "PhysicsManager.hpp"
#include "PhysicsScene.hpp"

#include <Core\Containers\Map.hpp>

#include "PhysicsActor.sgenerated.hpp"

using namespace physx;
using namespace D_CONTAINERS;


namespace Darius::Physics
{

	DUnorderedMap<PxActor*, PhysicsActor*> ActorMap;


	PhysicsActor::PhysicsActor(D_SCENE::GameObject const* gameObject, PhysicsActorType type) :
		mActorType(type),
		mPxActor(nullptr),
		mGameObject(gameObject)
	{
	}

	PhysicsActor::~PhysicsActor()
	{
		if (!mPxActor)
			return;
		auto scene = D_PHYSICS::GetScene();
		scene->mPxScene->removeActor(*mPxActor);
		ActorMap.erase(mPxActor);
		mPxActor = nullptr;
	}

	PhysicsActor* PhysicsActor::GetFromPxActor(physx::PxActor* actor)
	{
		if (!ActorMap.contains(actor))
			return nullptr;

		return ActorMap.at(actor);
	}

	void PhysicsActor::InitializeActor()
	{
		D_ASSERT(mGameObject);
		if (mPxActor)
			return;

		auto scene = D_PHYSICS::GetScene();
		auto physics = D_PHYSICS::GetCore();

		// Create actor initial transform
		auto mainTransform = mGameObject->GetTransform()->GetTransformData();
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

		ActorMap[mPxActor] = this;

		scene->mPxScene->addActor(*mPxActor);

	}
}
