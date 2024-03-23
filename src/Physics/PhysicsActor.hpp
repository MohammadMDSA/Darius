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
	class ColliderComponent;

	class PhysicsActor
	{
	public:

	public:
		PhysicsActor(D_SCENE::GameObject const* gameObject, PhysicsScene* scene);

		~PhysicsActor();
		
		static PhysicsActor*			GetFromPxActor(physx::PxActor* actor);

		INLINE bool						IsDynamic() const { return mDynamic; }

		INLINE physx::PxRigidActor*		GetPxActor() const { return mPxActor; }
		INLINE physx::PxRigidDynamic*	GetDynamicActor() const { return IsDynamic() ? reinterpret_cast<physx::PxRigidDynamic*>(mPxActor) : nullptr; }
		INLINE UINT						ColliderCount() const { return (UINT)mColliders.size(); }

		physx::PxShape*					AddCollider(ColliderComponent const* refComponent);
		void							RemoveCollider(ColliderComponent const* refComponent);
		physx::PxShape*					GetShape(std::string const& compName);
		
		template<typename T>
		physx::PxShape*					GetShape() { return GetShape(T::ClassName()); }

		void							PreUpdate();
		void							Update();

		void							ForceRemoveActor();

	private:
		friend class PhysicsScene;

		void							InitializeActor();
		void							UninitialzieActor();
		void							TransferShapes(physx::PxRigidActor* transfareShapes);
		bool							RemoveActorIfNecessary();
		
		physx::PxRigidActor*			mPxActor;

		const UINT						mDynamic : 1;
		UINT							mValid : 1;
		
		D_SCENE::GameObject const* const mGameObject;
		PhysicsScene* const				mScene;
		D_CONTAINERS::DUnorderedMap<physx::PxShape*, std::string> mColliders;
		D_CONTAINERS::DUnorderedMap<std::string, physx::PxShape*> mCollidersLookup;
	};

}

File_PhysicsActor_GENERATED
