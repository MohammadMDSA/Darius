#include "pch.hpp"
#include "PhysicsActor.hpp"

#include "PhysicsManager.hpp"
#include "PhysicsScene.hpp"

#include <Core\Containers\Map.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

#include "PhysicsActor.sgenerated.hpp"

using namespace physx;
using namespace D_CONTAINERS;


namespace Darius::Physics
{

	DUnorderedMap<PxActor*, PhysicsActor*> ActorMap;
	DSet<PhysicsActor*>	RequireDelete;


	PhysicsActor::PhysicsActor(D_SCENE::GameObject const* gameObject, PhysicsActorType type) :
		mActorType(type),
		mPxActor(nullptr),
		mGameObject(gameObject),
		mDirty(true)
	{
	}

	PhysicsActor::~PhysicsActor()
	{
		UninitialzieActor();

		RequireDelete.erase(this);

		if (!mPxActor)
			return;
		ActorMap.erase(mPxActor);
		mPxActor = nullptr;
	}

	void PhysicsActor::RemoveDeleted()
	{
		for (auto actor : RequireDelete)
		{
			for (auto collider : actor->mToBeRemoved)
			{
				actor->mCollider.erase(collider);
			}
			actor->mToBeRemoved.clear();
		}
		RequireDelete.clear();
	}

	PhysicsActor* PhysicsActor::GetFromPxActor(physx::PxActor* actor)
	{
		if (!ActorMap.contains(actor))
			return nullptr;

		return ActorMap.at(actor);
	}

	void PhysicsActor::UninitialzieActor()
	{
		mDirty = true;
		if (!mPxActor)
			return;
		auto scene = D_PHYSICS::GetScene();
		scene->mPxScene->removeActor(*mPxActor);
	}

	void PhysicsActor::RemoveCollider(void* shape)
	{
		mToBeRemoved.insert(reinterpret_cast<PxShape*>(shape));
		RequireDelete.insert(this);
	}

	void PhysicsActor::InitializeActor()
	{
		D_ASSERT(mGameObject);
		if (mPxActor && !mDirty)
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
		mCollider.clear();

		scene->mPxScene->addActor(*mPxActor);

		mDirty = false;

	}
}
