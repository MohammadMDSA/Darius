#pragma once

#include "Components/ColliderComponent.hpp"

#include <Core/Containers/Set.hpp>
#include <Scene/GameObject.hpp>
#include <Utils/Common.hpp>

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	class PhysicsScene;

	class PhysicsActor
	{
	public:
		enum class PhysicsActorType
		{
			Dynamic,
			Static,
			Kinematic
		};

	public:
		PhysicsActor(D_SCENE::GameObject* gameObject, PhysicsActorType type);
		~PhysicsActor();
		
		INLINE bool						IsDynamic() const { return mActorType == PhysicsActorType::Dynamic; }
		INLINE bool						IsKinematic() const { return mActorType == PhysicsActorType::Kinematic; }
		INLINE bool						IsStatic() const { return mActorType == PhysicsActorType::Static; }

		D_CH_R_FIELD(physx::PxRigidActor*, PxActor);
		D_CH_R_FIELD(const PhysicsActorType, ActorType);
	protected:
		void							InitializeActor();

	private:
		friend class PhysicsScene;

		D_SCENE::GameObject* const					mGameObject;
	};

}
