#pragma once

#include "Components/ColliderComponent.hpp"

#include <Core/Containers/Set.hpp>
#include <Scene/GameObject.hpp>
#include <Utils/Common.hpp>

#include "PhysicsActor.generated.hpp"

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	class PhysicsScene;

	class DClass(Serialize) PhysicsActor
	{
	public:

		enum class PhysicsActorType
		{
			Dynamic,
			Static,
			Kinematic
		};

		Darius_Physics_PhysicsActor_GENERATED

	public:
		PhysicsActor(D_SCENE::GameObject* gameObject, PhysicsActorType type);
		
		INLINE PhysicsActor() :
			mGameObject(nullptr),
			mPxActor(nullptr),
			mActorType(PhysicsActorType::Static) {}

		~PhysicsActor();
		
		INLINE bool						IsDynamic() const { return mActorType == PhysicsActorType::Dynamic; }
		INLINE bool						IsKinematic() const { return mActorType == PhysicsActorType::Kinematic; }
		INLINE bool						IsStatic() const { return mActorType == PhysicsActorType::Static; }

	private:
		friend class PhysicsScene;

		void							InitializeActor();
		

		
		DField(Get[const, inline])
		physx::PxRigidActor*			mPxActor;

		DField(Get[inline])
		const PhysicsActorType			mActorType;
		
		D_SCENE::GameObject* const		mGameObject;
		D_CONTAINERS::DSet<D_CORE::Uuid, boost::hash<D_CORE::Uuid>> mCollider;
	};

}

File_PhysicsActor_GENERATED
