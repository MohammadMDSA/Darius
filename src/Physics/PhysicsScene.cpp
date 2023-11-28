#include "pch.hpp"
#include "PhysicsScene.hpp"

#include "PhysicsManager.hpp"
#include "Physics/PhysicsActor.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/RigidbodyComponent.hpp"

#include <Debug/DebugDraw.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>

#include <PxPhysics.h>
#include <PxPhysicsAPI.h>

using namespace D_SCENE;

using namespace physx;

#define PX_RELEASE(x)	if(x) { x->release(); x = NULL; }

namespace Darius::Physics
{

	PhysicsScene::PhysicsScene(PxScene* scene) :
		mPxScene(scene)
	{

	}

	PhysicsScene::~PhysicsScene()
	{
		PX_RELEASE(mPxScene);
	}

	void PhysicsScene::Simulate(bool fetchResults, float deltaTime)
	{
		D_PROFILING::ScopedTimer physicsProfiler(L"Physics Simulation Update");
		mPxScene->simulate(deltaTime);
		mPxScene->fetchResults(fetchResults);
	}

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

		if (!actor.mPxActor)
			actor.InitializeActor();
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
		auto ref = go;
		if (sActorMap.contains(ref))
			return;

		sActorMap.emplace(ref, PhysicsActor(ref, PhysicsActor::PhysicsActorType::Static));
		sActorMap[ref].InitializeActor();
	}


	bool PhysicsScene::CastRay(_IN_ D_MATH::Vector3 const& origin, _IN_ D_MATH::Vector3 const& direction, _IN_ float maxDistance, _OUT_ physx::PxRaycastBuffer& hit)
	{
		return mPxScene->raycast(*(PxVec3 const*)&origin, *(PxVec3 const*)&direction, maxDistance, hit);
	}

	bool PhysicsScene::CastCapsule(_IN_ D_MATH::Vector3 const& origin, _IN_ D_MATH::Vector3 const& direction, float radius, float halfHeight, D_MATH::Quaternion const& capsuleRotation, float maxDistance, physx::PxSweepBuffer& hit)
	{
		D_ASSERT(radius > 0);
		D_ASSERT(halfHeight > 0);
		return mPxScene->sweep(PxCapsuleGeometry(radius, halfHeight), PxTransform(GetVec3(origin), GetQuat(capsuleRotation)), GetVec3(direction.Normalize()), maxDistance, hit);
	}

	bool PhysicsScene::CastSphere(D_MATH::Vector3 const& origin, D_MATH::Vector3 const& direction, float radius, float maxDistance, _OUT_ physx::PxSweepBuffer& hit)
	{
		D_ASSERT(radius > 0);
		return mPxScene->sweep(PxSphereGeometry(radius), PxTransform(GetVec3(origin)), GetVec3(direction.Normalize()), maxDistance, hit);
	}

	bool PhysicsScene::CastBox(D_MATH::Vector3 const& origin, D_MATH::Vector3 const& direction, D_MATH::Vector3 const& halfExtents, D_MATH::Quaternion const& boxRotation, float maxDistance, _OUT_ physx::PxSweepBuffer& hit)
	{
		D_ASSERT(DirectX::XMVector3Greater(halfExtents, DirectX::XMVectorZero()));

		return mPxScene->sweep(PxBoxGeometry(GetVec3(halfExtents)), PxTransform(GetVec3(origin), GetQuat(boxRotation)), GetVec3(direction.Normalize()), maxDistance, hit);
	}

	bool PhysicsScene::CastRay_DebugDraw(_IN_ D_MATH::Vector3 const& origin, _IN_ D_MATH::Vector3 const& direction, _IN_ float maxDistance, float secendsToDisplay, _OUT_ physx::PxRaycastBuffer& hit)
	{
		bool result = CastRay(origin, direction, maxDistance, hit);

#if _D_EDITOR
		if (result)
		{
			auto hitPos = GetVec3(hit.block.position);
			D_DEBUG_DRAW::DrawLine(origin, hitPos, secendsToDisplay, D_MATH::Color::Green);
			D_DEBUG_DRAW::DrawLine(hitPos, GetVec3(hit.block.normal) * 0.5f + hitPos, secendsToDisplay, D_MATH::Color::Red);
		}

		else
			D_DEBUG_DRAW::DrawLine(origin, direction.Normalize() * maxDistance + origin, secendsToDisplay, D_MATH::Color::Red);
#endif
		return result;

	}

	bool PhysicsScene::CastCapsule_DebugDraw(_IN_ D_MATH::Vector3 const& origin, _IN_ D_MATH::Vector3 const& direction, float maxDistance, float radius, float halfHeight, D_MATH::Quaternion const& capsuleRotation, float secendsToDisplay, _OUT_ physx::PxSweepBuffer& hit)
	{
		bool result = CastCapsule(origin, direction, radius, halfHeight, capsuleRotation, maxDistance, hit);

#if _D_EDITOR

		auto dirNorm = direction.Normalize();

		if (result)
		{
			auto hitGeomPos = origin + dirNorm * hit.block.distance;
			D_DEBUG_DRAW::DrawLine(origin, hitGeomPos, secendsToDisplay, D_MATH::Color::Green);
			D_DEBUG_DRAW::DrawCapsule(hitGeomPos, radius, halfHeight, capsuleRotation, D_DEBUG_DRAW::CapsuleOrientation::AlongX, 16u, secendsToDisplay, D_MATH::Color::Red);
		}

		else
			D_DEBUG_DRAW::DrawLine(origin, dirNorm * maxDistance + origin, secendsToDisplay, D_MATH::Color::Red);
#endif
		return result;
	}

	bool PhysicsScene::CastSphere_DebugDraw(D_MATH::Vector3 const& origin, D_MATH::Vector3 const& direction, float radius, float maxDistance, float secendsToDisplay, _OUT_ physx::PxSweepBuffer& hit)
	{
		bool result = CastSphere(origin, direction, radius, maxDistance, hit);

#if _D_EDITOR

		auto dirNorm = direction.Normalize();

		if (result)
		{
			auto hitGeomPos = origin + dirNorm * hit.block.distance;
			D_DEBUG_DRAW::DrawLine(origin, hitGeomPos, secendsToDisplay, D_MATH::Color::Green);
			D_DEBUG_DRAW::DrawSphere(hitGeomPos, radius, secendsToDisplay, D_MATH::Color::Red);
		}

		else
			D_DEBUG_DRAW::DrawLine(origin, dirNorm * maxDistance + origin, secendsToDisplay, D_MATH::Color::Red);
#endif
		return result;
	}

	bool PhysicsScene::CastBox_DebugDraw(D_MATH::Vector3 const& origin, D_MATH::Vector3 const& direction, D_MATH::Vector3 const& halfExtents, D_MATH::Quaternion const& boxRotation, float maxDistance, float secendsToDisplay, _OUT_ physx::PxSweepBuffer& hit)
	{
		bool result = CastBox(origin, direction, halfExtents, boxRotation, maxDistance, hit);


#if _D_EDITOR

		auto dirNorm = direction.Normalize();

		if (result)
		{
			auto hitGeomPos = origin + dirNorm * hit.block.distance;
			D_DEBUG_DRAW::DrawLine(origin, hitGeomPos, secendsToDisplay, D_MATH::Color::Green);
			D_DEBUG_DRAW::DrawCube(hitGeomPos, boxRotation, halfExtents * 2, secendsToDisplay, D_MATH::Color::Red);
		}

		else
			D_DEBUG_DRAW::DrawLine(origin, dirNorm * maxDistance + origin, secendsToDisplay, D_MATH::Color::Red);
#endif
		return result;
	}

}
