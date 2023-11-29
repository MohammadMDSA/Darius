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

// Detects a trigger using the shape's simulation filter data. See createTriggerShape() function.
bool isTrigger(const PxFilterData & data)
{
	if (data.word0 != 0xffffffff)
		return false;
	if (data.word1 != 0xffffffff)
		return false;
	if (data.word2 != 0xffffffff)
		return false;
	if (data.word3 != 0xffffffff)
		return false;
	return true;
}

namespace Darius::Physics
{

	PhysicsScene::PhysicsScene(PxSceneDesc const& sceneDesc, PxPhysics* core)
	{
		PxSceneDesc desc = sceneDesc;
		desc.filterShader = PxDefaultSimulationFilterShader;
		desc.simulationEventCallback = &mCallbacks;

		mPxScene = core->createScene(desc);

#ifdef _DEBUG
		PxPvdSceneClient* pvdClient = mPxScene->getScenePvdClient();
		if (pvdClient)
		{
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}
#endif // _DEBUG

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
			actor.mCollider.emplace(shape, collider->GetComponentName());

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
		auto shape = collider->mShape;

		pxActor->detachShape(*shape);

		actor.mCollider.erase(shape);

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

	void PhysicsScene::SimulationCallback::onContact(physx::PxContactPairHeader const& pairHeader, physx::PxContactPair const* pairs, physx::PxU32 nbPairs)
	{
		for (UINT i = 0u; i < nbPairs; i++)
		{
			// Finding physics actors
			auto& pair = pairs[i];
			auto actor1 = PhysicsActor::GetFromPxActor(pairHeader.actors[0]);
			auto actor2 = PhysicsActor::GetFromPxActor(pairHeader.actors[1]);

			// Finding collider component names which generated the event
			auto compName1 = actor1->mCollider.at(pair.shapes[0]);
			auto compName2 = actor2->mCollider.at(pair.shapes[1]);

			// Finding the corresponding game objects
			auto go1 = actor1->mGameObject;
			auto go2 = actor2->mGameObject;

			// Finding the components responsible for the collision
			auto comp1 = reinterpret_cast<ColliderComponent*>(go1->GetComponent(compName1));
			auto comp2 = reinterpret_cast<ColliderComponent*>(go2->GetComponent(compName2));

			D_STATIC_ASSERT(sizeof(PxVec3) == 3 * sizeof(float));
			
			// Loading hit points
			HitResult hit;
			D_CONTAINERS::DVector<PxContactPairPoint> nativePoints;
			UINT contactCount = pair.contactCount;
			if (contactCount > 0)
			{
				nativePoints.resize(contactCount);
				for (UINT i = 0u; i < contactCount; i++)
				{
					hit.Contacts.push_back(
						{
						D_MATH::Vector3(&nativePoints[i].position.x),
						nativePoints[i].separation,
						D_MATH::Vector3(&nativePoints[i].normal.x),
						D_MATH::Vector3(&nativePoints[i].impulse.x),
						}
					);
				}
			}

			// Firing contact event
			if (pair.events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				comp1->OnColliderContactEnter(comp1, comp2, const_cast<D_SCENE::GameObject*>(go2), hit);
				comp2->OnColliderContactEnter(comp2, comp1, const_cast<D_SCENE::GameObject*>(go1), hit);
			}

			// Firing stay event
			if (pair.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
			{
				comp1->OnColliderContactStay(comp1, comp2, const_cast<D_SCENE::GameObject*>(go2), hit);
				comp2->OnColliderContactStay(comp2, comp1, const_cast<D_SCENE::GameObject*>(go1), hit);
			}

			// Firing lost event
			if (pair.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
			{
				comp1->OnColliderContactLost(comp1, comp2, const_cast<D_SCENE::GameObject*>(go2), hit);
				comp2->OnColliderContactLost(comp2, comp1, const_cast<D_SCENE::GameObject*>(go1), hit);
			}
		}
	}

	void PhysicsScene::SimulationCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
	{
		for (UINT i = 0u; i < count; i++)
		{
			// Finding physics actors
			auto& pair = pairs[i];
			auto actor1 = PhysicsActor::GetFromPxActor(pair.triggerActor);
			auto actor2 = PhysicsActor::GetFromPxActor(pair.otherActor);

			// Finding collider component names which generated the event
			auto compName1 = actor1->mCollider.at(pair.triggerShape);

			// Finding the corresponding game objects
			auto go1 = actor1->mGameObject;
			auto go2 = actor2->mGameObject;

			// Finding the components responsible for the collision
			auto comp1 = reinterpret_cast<ColliderComponent*>(go1->GetComponent(compName1));

			// Firing trigger enter event
			if (pair.status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				comp1->OnTriggerEnter(comp1, const_cast<D_SCENE::GameObject*>(go2));
			}

			// Firing trigger exit event
			if (pair.status & PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				comp1->OnTriggerExit(comp1, const_cast<D_SCENE::GameObject*>(go2));
			}

		}
	}

}
