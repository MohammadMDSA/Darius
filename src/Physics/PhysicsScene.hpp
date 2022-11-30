#pragma once

#include "PhysicsActor.hpp"

#include <Core/Signal.hpp>
#include <Scene/GameObject.hpp>

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	class ColliderComponent;
	class RigidbodyComponent;

	class PhysicsScene
	{
	private:

		friend class ColliderComponent;
		friend class RigidbodyComponent;

		static physx::PxRigidDynamic*	AddDynamicActor(D_SCENE::GameObject* go, bool kinematic);
		static void						RemoveDynamicActor(D_SCENE::GameObject* go);
		static physx::PxShape*			AddCollider(ColliderComponent* collider, _OUT_ bool& nonStatic);
		static void						RemoveCollider(ColliderComponent const* collider);
		static void						AddActor(D_SCENE::GameObject const* go);
		static INLINE bool				IsRegistered(D_SCENE::GameObject const* go) { return sActorMap.contains(const_cast<D_SCENE::GameObject*>(go)); }

		static void						InvalidateAllColliders(D_SCENE::GameObject const* go);

		static D_CONTAINERS::DUnorderedMap<D_SCENE::GameObject*, PhysicsActor> sActorMap;

	};

}
