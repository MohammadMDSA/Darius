#include "pch.hpp"
#include "PhysicsScene.hpp"

#include "PhysicsManager.hpp"
#include "Physics/PhysicsActor.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/RigidbodyComponent.hpp"

#include <PxPhysics.h>
#include <PxPhysicsAPI.h>

using namespace D_SCENE;

using namespace physx;

namespace Darius::Physics
{

	D_CONTAINERS::DUnorderedMap<D_SCENE::GameObject*, PhysicsActor> PhysicsScene::sActorMap = {};

	PxRigidDynamic* PhysicsScene::AddDynamicActor(D_SCENE::GameObject* go, bool kinematic)
	{
		// An actor is already registered for the corresponding game object
		if (sActorMap.contains(go))
		{
			auto& actor = sActorMap[go];

			// The registered actor is a dynamic one so just return it
			if (!actor.IsStatic())
				return reinterpret_cast<PxRigidDynamic*>(actor.mPxActor);

			sActorMap.erase(go);
		}

		// Deciding the actor type
		auto type = kinematic ? PhysicsActor::PhysicsActorType::Kinematic : PhysicsActor::PhysicsActorType::Dynamic;
		sActorMap.emplace(go, PhysicsActor(go, type));
		auto& newActor = sActorMap[go];
		newActor.InitializeActor(); // Initialize physics actor
		auto dynamicActor = reinterpret_cast<PxRigidDynamic*>(newActor.mPxActor);

		return dynamicActor;

	}

	void PhysicsScene::RemoveDynamicActor(D_SCENE::GameObject* go)
	{
		// If there is no actor for the gameobject, we don't have to do anything
		if (!sActorMap.contains(go))
			return;

		// So there is an actor
		auto& actor = sActorMap[go];

		// Don't do anything if it already is static
		if (actor.IsStatic())
			return;

		auto invalidCollidersCount = actor.mCollider.size();

		sActorMap.erase(go);

		if (invalidCollidersCount <= 0)
			return;

		sActorMap.emplace(go, PhysicsActor(go, PhysicsActor::PhysicsActorType::Static));
		auto& newActor = sActorMap[go];

	}

	physx::PxShape* PhysicsScene::AddCollider(ColliderComponent* collider, _OUT_ bool& nonStatic, PhysicsActor** physicsActor)
	{
		auto go = collider->GetGameObject();

		if (!sActorMap.contains(go))
			AddActor(go);

		D_ASSERT(sActorMap.contains(go));

		auto& actor = sActorMap[go];

		// Check if corresponding actor is non-static
		nonStatic = !actor.IsStatic();

		auto pxActor = actor.mPxActor;

		auto shape = PxRigidActorExt::createExclusiveShape(*pxActor, *collider->GetPhysicsGeometry(), *collider->GetMaterial());

		if (shape)
			actor.mCollider.insert(collider->GetUuid());

		if (physicsActor)
			*physicsActor = &actor;

		return shape;

	}

	void PhysicsScene::RemoveCollider(ColliderComponent const* collider)
	{
		auto coll = const_cast<ColliderComponent*>(collider);
		auto go = coll->GetGameObject();

		if (!sActorMap.contains(go))
			return;

		auto& actor = sActorMap[go];

		auto pxActor = actor.mPxActor;

		pxActor->detachShape(*collider->mShape);

		actor.mCollider.erase(collider->GetUuid());

		if (actor.IsStatic() && actor.mCollider.size() <= 0)
		{
			sActorMap.erase(go);
		}
	}

	void PhysicsScene::AddActor(GameObject const* go)
	{
		auto ref = const_cast<GameObject*>(go);
		if (sActorMap.contains(ref))
			return;

		sActorMap.emplace(ref, PhysicsActor(ref, PhysicsActor::PhysicsActorType::Static));
		sActorMap[ref].InitializeActor();
	}

}
