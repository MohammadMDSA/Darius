#pragma once

#include "PhysicsActor.hpp"

#include <Core/Memory/Allocators/PagedAllocator.hpp>
#include <Core/Signal.hpp>
#include <Scene/GameObject.hpp>
#include <Utils/BuildWarnings.hpp>

D_H_WARNING_SCOPE_BEGIN()
D_H_WARNING_DISABLE(4435)
#include <PxPhysicsAPI.h>
D_H_WARNING_SCOPE_END()


#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	class ColliderComponent;
	class RigidbodyComponent;

	class PhysicsScene : public NonCopyable
	{
	public:
		PhysicsScene(physx::PxSceneDesc const& sceneDesc, physx::PxPhysics* core);
		~PhysicsScene();

	// Scene Queries
		bool					CastRay(_IN_ D_MATH::Vector3 const& origin, _IN_ D_MATH::Vector3 const& direction, _IN_ float maxDistance, _OUT_ physx::PxRaycastBuffer& hit);
		bool					CastCapsule(_IN_ D_MATH::Vector3 const& origin, _IN_ D_MATH::Vector3 const& direction, float radius, float halfHeight, D_MATH::Quaternion const& capsuleRotation, float maxDistance, _OUT_ physx::PxSweepBuffer& hit);
		bool					CastSphere(D_MATH::Vector3 const& origin, D_MATH::Vector3 const& direction, float radius, float maxDistance, _OUT_ physx::PxSweepBuffer& hit);
		bool					CastBox(D_MATH::Vector3 const& origin, D_MATH::Vector3 const& direction, D_MATH::Vector3 const& halfExtents, D_MATH::Quaternion const& boxRotation, float maxDistance, _OUT_ physx::PxSweepBuffer& hit);

		bool					CastRay_DebugDraw(_IN_ D_MATH::Vector3 const& origin, _IN_ D_MATH::Vector3 const& direction, _IN_ float maxDistance, float secendsToDisplay, _OUT_ physx::PxRaycastBuffer& hit);
		bool					CastCapsule_DebugDraw(_IN_ D_MATH::Vector3 const& origin, _IN_ D_MATH::Vector3 const& direction, float maxDistance, float radius, float halfHeight, D_MATH::Quaternion const& capsuleRotation, float secendsToDisplay, _OUT_ physx::PxSweepBuffer& hit);
		bool					CastSphere_DebugDraw(D_MATH::Vector3 const& origin, D_MATH::Vector3 const& direction, float radius, float maxDistance, float secendsToDisplay, _OUT_ physx::PxSweepBuffer& hit);
		bool					CastBox_DebugDraw(D_MATH::Vector3 const& origin, D_MATH::Vector3 const& direction, D_MATH::Vector3 const& halfExtents, D_MATH::Quaternion const& boxRotation, float maxDistance, float secendsToDisplay, _OUT_ physx::PxSweepBuffer& hit);

		bool					Simulate(bool simulating, bool fetchResults, float deltaTime);
		void					PreUpdate();
		void					UpdateControllers(float dt);
		void					Update();
		INLINE D_MATH::Vector3	GetGravityVector() const { return mGravityVec; }

		physx::PxController*	CreateController(physx::PxControllerDesc const& controllerDesc);
		bool					ReleaseController(physx::PxController* controller);

		PhysicsActor const*		FindPhysicsActor(D_SCENE::GameObject* go) const;
		PhysicsActor*			FindPhysicsActor(D_SCENE::GameObject* go);
		PhysicsActor*			FindOrCreatePhysicsActor(D_SCENE::GameObject* go);

	private:

		friend class ColliderComponent;
		friend class RigidbodyComponent;
		friend class PhysicsActor;

		class SimulationCallback : public physx::PxSimulationEventCallback
		{
			virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override { }
			virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override { }
			virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) { }
			virtual void onContact(physx::PxContactPairHeader const& pairHeader, physx::PxContactPair const* pairs, physx::PxU32 nbPairs) override;
			virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
			virtual void onAdvance(physx::PxRigidBody const* const* bodyBuffer, physx::PxTransform const* poseBuffer, const physx::PxU32 count) override { }

		};
		void					RemoveActor(PhysicsActor* actor);

		D_CONTAINERS::DUnorderedMap<D_SCENE::GameObject const*, PhysicsActor*> mActorMap;
		D_CONTAINERS::DSet<physx::PxController*>	mControllers;

		physx::PxScene*								mPxScene;
		physx::PxControllerManager*					mControllerManager;
		SimulationCallback							mCallbacks;
		D_MATH::Vector3								mGravityVec;
		D_MEMORY::PagedAllocator<PhysicsActor>		mActorAllocator;

		float										mTimeStepAccumulator;
		constexpr static float						sTimeStep = 1.f / 60.f;
	};

}
