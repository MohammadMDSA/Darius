#pragma once

#include <Core/Containers/Set.hpp>
#include <Scene/GameObject.hpp>
#include <Utils/Common.hpp>

#include <PxActor.h>

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
		PhysicsActor(D_SCENE::GameObject const* gameObject, PhysicsActorType type);
		
		INLINE PhysicsActor() :
			mGameObject(nullptr),
			mPxActor(nullptr),
			mActorType(PhysicsActorType::Static) {}

		~PhysicsActor();
		
		static PhysicsActor*			GetFromPxActor(physx::PxActor* actor);

		INLINE bool						IsDynamic() const { return mActorType == PhysicsActorType::Dynamic; }
		INLINE bool						IsKinematic() const { return mActorType == PhysicsActorType::Kinematic; }
		INLINE bool						IsStatic() const { return mActorType == PhysicsActorType::Static; }
		INLINE physx::PxRigidActor*		GetPxActor() const { return mPxActor; }
		INLINE PhysicsActorType 		GetActorType() const { return mActorType; }
		INLINE int						ColliderCount() const { return (int)mCollider.size() - (int)mToBeRemoved.size(); }

	private:
		friend class PhysicsScene;

		void							InitializeActor();
		void							UninitialzieActor();
		void							RemoveCollider(void* shape);
		static void						RemoveDeleted();
		
		DField()
		physx::PxRigidActor*			mPxActor;

		DField()
		const PhysicsActorType			mActorType;

		bool							mDirty;
		
		D_SCENE::GameObject const* const mGameObject;
		D_CONTAINERS::DUnorderedMap<physx::PxShape*, std::string> mCollider;
		D_CONTAINERS::DSet<physx::PxShape*> mToBeRemoved;
	};

}

File_PhysicsActor_GENERATED
