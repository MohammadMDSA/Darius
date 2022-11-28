#pragma once

#include "PhysicsActor.hpp"

#include <Scene/GameObject.hpp>

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	class PhysicsScene
	{
	public:

		static physx::PxRigidDynamic*	AddDynamicActor(D_SCENE::GameObject* go, bool kinematic);
		static void						RemoveDynamicActor(D_SCENE::GameObject* go);
		static physx::PxShape*			AddShapeToActor(D_SCENE::GameObject const* go, physx::PxGeometry const* geom, _OUT_ bool& nonStatic);
		static void						RemoveShapeFromActor(D_SCENE::GameObject const* go, physx::PxShape const* shape);
		static INLINE bool				IsRegistered(D_SCENE::GameObject const* go) { return sActorMap.contains(const_cast<D_SCENE::GameObject*>(go)); }

	private:

		static void						DynamicActorToStatic(physx::PxRigidDynamic const* dynamic, physx::PxRigidStatic* staticActor);
		static void						StaticActorToDynamic(physx::PxRigidStatic const* staticActor, physx::PxRigidDynamic* dynamic);

		static D_CONTAINERS::DUnorderedMap<D_SCENE::GameObject*, PhysicsActor> sActorMap;

	};

}
