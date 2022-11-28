#include "pch.hpp"
#include "PhysicsScene.hpp"

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
			if (actor.IsDynamic())
				return dynamic_cast<PxRigidDynamic*>(actor.GetPxActor());


			// The registered one is an static one and needs to get switched to a dynamic one
			auto staticActor = dynamic_cast<PxRigidStatic*>(actor.GetPxActor());
			sActorMap.erase(go);

			// Deciding the actor type
			auto type = kinematic ? PhysicsActor::PhysicsActorType::Kinematic : PhysicsActor::PhysicsActorType::Dynamic;
			sActorMap.emplace(go, PhysicsActor(go, type));
			auto& newActor = sActorMap[go];
			newActor.InitializeActor(); // Initialize physics actor
			auto dynamicActor = dynamic_cast<PxRigidDynamic*>(newActor.GetPxActor());

			StaticActorToDynamic(staticActor, dynamicActor);

			return dynamicActor;
		}
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

		auto dynamicActor = dynamic_cast<PxRigidDynamic*>(actor.GetPxActor());
		sActorMap.erase(go);
		sActorMap.emplace(go, PhysicsActor(go, PhysicsActor::PhysicsActorType::Static));
		auto& newActor = sActorMap[go];
		auto staticActor = dynamic_cast<PxRigidStatic*>(actor.GetPxActor());

		DynamicActorToStatic(dynamicActor, staticActor);
	}

	physx::PxShape* PhysicsScene::AddShapeToActor(D_SCENE::GameObject const* go, physx::PxGeometry const* geom, _OUT_ bool& nonStatic)
	{
		auto ref = const_cast<D_SCENE::GameObject*>(go);
		auto& actor = sActorMap[ref];

		// Check if corresponding actor is non-static
		nonStatic = !actor.IsStatic();



	}
	
}
