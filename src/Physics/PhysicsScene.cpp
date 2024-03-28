#include "pch.hpp"
#include "PhysicsScene.hpp"

#include "PhysicsManager.hpp"
#include "Physics/PhysicsActor.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/RigidbodyComponent.hpp"

#include <Debug/DebugDraw.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>
#include <Job/Job.hpp>

#include <PxPhysics.h>
#include <PxPhysicsAPI.h>

using namespace D_SCENE;

using namespace physx;

#define PX_RELEASE(x)	if(x) { x->release(); x = NULL; }

bool IsTrigger(PxFilterData const& data)
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

PxFilterFlags triggersUsingFilterShader(PxFilterObjectAttributes /*attributes0*/, PxFilterData filterData0,
	PxFilterObjectAttributes /*attributes1*/, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* /*constantBlock*/, PxU32 /*constantBlockSize*/)
{
	// We need to detect whether one of the shapes is a trigger.
	const bool isTriggerPair = IsTrigger(filterData0) || IsTrigger(filterData1);

	// If we have a trigger, replicate the trigger codepath from PxDefaultSimulationFilterShader
	if (isTriggerPair)
	{
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;

		//if (usesCCD())
		//	pairFlags |= PxPairFlag::eDETECT_CCD_CONTACT;

		return PxFilterFlag::eDEFAULT;
	}
	else
	{
		// Otherwise use the default flags for regular pairs
		pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_LOST | PxPairFlag::eNOTIFY_TOUCH_PERSISTS;
		return PxFilterFlag::eDEFAULT;
	}
}

namespace Darius::Physics
{

	PhysicsScene::PhysicsScene(PxSceneDesc const& sceneDesc, PxPhysics* core, bool gpuAccelerated)
	{
		PxSceneDesc desc = sceneDesc;
		desc.filterShader = triggersUsingFilterShader;
		desc.simulationEventCallback = &mCallbacks;

		if (gpuAccelerated)
			desc.broadPhaseType = PxBroadPhaseType::eGPU;

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

		if (gpuAccelerated)
			mPxScene->setFlag(PxSceneFlag::eENABLE_GPU_DYNAMICS, true);

	}

	PhysicsScene::~PhysicsScene()
	{
		PX_RELEASE(mPxScene);
	}

	void PhysicsScene::PreUpdate()
	{
		D_PROFILING::ScopedTimer _prof(L"Physics Simulation Pre Update");
		for (auto& [_, actor] : mActorMap)
		{
			actor.PreUpdate();
		}
	}

	void PhysicsScene::Update()
	{
		D_PROFILING::ScopedTimer physicsProfiler(L"Physics Post Update");

		UINT numRigidBodyComps = (UINT)mActorMap.size();
		if (numRigidBodyComps <= 0)
			return;

		D_CONTAINERS::DVector<std::function<void()>> updateFuncs;
		updateFuncs.reserve(numRigidBodyComps);

		for (auto& [_, actor] : mActorMap)
		{

			if (actor.IsDynamic())
			{
				updateFuncs.push_back([&]()
					{
						actor.Update();

					});
			}
		}

		D_JOB::AddTaskSetAndWait(updateFuncs);
	}

	void PhysicsScene::Simulate(bool fetchResults, float deltaTime)
	{
		D_PROFILING::ScopedTimer physicsProfiler(L"Physics Simulation Update");

		PreUpdate();

		mPxScene->simulate(deltaTime);
		mPxScene->fetchResults(fetchResults);

		Update();
	}

	PhysicsActor const* PhysicsScene::FindPhysicsActor(GameObject* go) const
	{
		if (!mActorMap.contains(go))
			return nullptr;

		return &mActorMap.at(go);
	}

	PhysicsActor* PhysicsScene::FindPhysicsActor(GameObject* go)
	{
		if (!mActorMap.contains(go))
			return nullptr;

		return &mActorMap.at(go);
	}

	PhysicsActor* PhysicsScene::FindOrCreatePhysicsActor(GameObject* go)
	{
		auto result = FindPhysicsActor(go);

		if (result)
			return result;

		// Create and initialize the actor
		mActorMap.emplace(go, PhysicsActor(go, this));
		result = &mActorMap.at(go);
		result->InitializeActor();
		return result;
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

			if (!actor1 || !actor2)
				continue;

			// Finding collider component names which generated the event
			auto compName1 = actor1->mColliders.at(pair.shapes[0]);
			auto compName2 = actor2->mColliders.at(pair.shapes[1]);

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
				if (comp1->IsActive())
					comp1->OnColliderContactEnter(comp1, comp2, const_cast<D_SCENE::GameObject*>(go2), hit);

				if (comp2->IsActive())
					comp2->OnColliderContactEnter(comp2, comp1, const_cast<D_SCENE::GameObject*>(go1), hit);
			}

			// Firing stay event
			if (pair.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
			{
				if (comp1->IsActive())
					comp1->OnColliderContactStay(comp1, comp2, const_cast<D_SCENE::GameObject*>(go2), hit);

				if (comp2->IsActive())
					comp2->OnColliderContactStay(comp2, comp1, const_cast<D_SCENE::GameObject*>(go1), hit);
			}

			// Firing lost event
			if (pair.events & PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				if (comp1->IsActive())
					comp1->OnColliderContactLost(comp1, comp2, const_cast<D_SCENE::GameObject*>(go2), hit);

				if (comp2->IsActive())
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

			if (!actor1 || !actor2)
				continue;

			// Finding collider component names which generated the event
			auto compName1 = actor1->mColliders.at(pair.triggerShape);

			// Finding the corresponding game objects
			auto go1 = actor1->mGameObject;
			auto go2 = actor2->mGameObject;

			// Finding the components responsible for the collision
			auto comp1 = reinterpret_cast<ColliderComponent*>(go1->GetComponent(compName1));

			// Firing trigger enter event
			if (pair.status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				if (comp1->IsActive())
					comp1->OnTriggerEnter(comp1, const_cast<D_SCENE::GameObject*>(go2));
			}

			// Firing trigger exit event
			if (pair.status & PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				if (comp1->IsActive())
					comp1->OnTriggerExit(comp1, const_cast<D_SCENE::GameObject*>(go2));
			}

		}
	}

	void PhysicsScene::RemoveActor(PhysicsActor* actor)
	{
		auto pxActor = actor->mPxActor;

		mActorMap.erase(actor->mGameObject);
	}
}
