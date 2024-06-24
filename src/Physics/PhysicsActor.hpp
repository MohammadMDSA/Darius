#pragma once

#include <Core/Containers/Set.hpp>
#include <Core/RefCounting/Counted.hpp>
#include <Scene/GameObject.hpp>
#include <Utils/Common.hpp>

#include <PxActor.h>
#include <geometry/PxGeometry.h>

#include "PhysicsActor.generated.hpp"

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	class PhysicsScene;
	class ColliderComponent;

	class PhysicsActor : public D_CORE::Counted
	{
	public:

	public:
		PhysicsActor(D_SCENE::GameObject const* gameObject, PhysicsScene* scene);

		~PhysicsActor();
		
		static PhysicsActor*			GetFromPxActor(physx::PxActor* actor);

		INLINE bool						IsValid() const { return mValid; }
		INLINE bool						IsDynamic() const { return mDynamic; }
		void							SetDynamic(bool dynamic);

		INLINE physx::PxRigidActor*		GetPxActor() const { return mPxActor; }
		INLINE physx::PxRigidDynamic*	GetDynamicActor() const { return IsDynamic() ? reinterpret_cast<physx::PxRigidDynamic*>(mPxActor) : nullptr; }
		INLINE UINT						ColliderCount() const { return (UINT)mColliders.size(); }

		physx::PxShape*					AddCollider(ColliderComponent const* refComponent);
		void							RemoveCollider(ColliderComponent const* refComponent);
		physx::PxShape*					GetShape(D_CORE::StringId const& compName);
		
		template<typename T>
		physx::PxShape*					GetShape() { return GetShape(T::ClassName()); }

		void							PreUpdate();
		void							Update();

		bool							IsGeometryCompatible(physx::PxGeometryType::Enum type);

		void							InitializeActor();

		INLINE void						SetActorTransformDirty() { mTransformDirty = true; }
		INLINE bool						IsActorTransfromDirty() const { return mTransformDirty; }

		virtual bool					Release() override;
	private:
		friend class PhysicsScene;

		void							SetActorTransformDirtyCallback(D_MATH::TransformComponent* transComp, D_MATH::Transform const& newTrans);


		void							UninitialzieActor();
		void							TransferShapes(physx::PxRigidActor* transfareShapes);
		bool							RemoveActorIfNecessary();
		
		physx::PxRigidActor*			mPxActor;

		UINT							mDynamic : 1;
		UINT							mValid : 1;
		UINT							mDynamicDirty : 1;
		UINT							mTransformDirty : 1;
		
		D_SCENE::GameObject const* const mGameObject;
		PhysicsScene* const				mScene;
		D_CONTAINERS::DUnorderedMap<physx::PxShape*, D_CORE::StringId> mColliders;
		D_CONTAINERS::DUnorderedMap<D_CORE::StringId, physx::PxShape*> mCollidersLookup;

		D_CORE::SignalConnection		mTransformChangeConnection;
	};

}

File_PhysicsActor_GENERATED
