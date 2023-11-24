#include "pch.hpp"
#include "PhysicsManager.hpp"

#include "Components/RigidbodyComponent.hpp"
#include "Components/BoxColliderComponent.hpp"
#include "Components/SphereColliderComponent.hpp"
#include "Components/CapsuleColliderComponent.hpp"
#include "Resources/PhysicsMaterialResource.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>
#include <Job/Job.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Scene/Scene.hpp>
#include <Utils/Assert.hpp>

#ifdef _D_EDITOR
#include <Debug/DebugDraw.hpp>
#endif

#include <PxPhysics.h>

#define PX_RELEASE(x)	if(x)	{ x->release(); x = NULL;	}

using namespace physx;

namespace Darius::Physics
{
	bool									_init = false;

	PxDefaultAllocator						gAllocator;
	PxDefaultErrorCallback					gErrorCallback;
	PxTolerancesScale						gToleranceScale;

	PxFoundation* gFoundation = NULL;
	PxPhysics* gPhysics = NULL;

	PxDefaultCpuDispatcher* gDispatcher = NULL;
	PxScene* gScene = NULL;

	PxPvd* gPvd = NULL;

	D_RESOURCE::ResourceHandle				gDefaultMaterial;

	void					UpdatePostPhysicsTransforms();
	void					UpdatePrePhysicsTransform(bool simulating);

	void Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(!_init);
		_init = true;

		gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

#ifdef _DEBUG
		gPvd = PxCreatePvd(*gFoundation);
		PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
#endif

		gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, gToleranceScale, true, gPvd);

		PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
		sceneDesc.gravity = PxVec3(0.f, -9.8f, 0.f);
		gDispatcher = PxDefaultCpuDispatcherCreate(2);
		sceneDesc.cpuDispatcher = gDispatcher;
		sceneDesc.filterShader = PxDefaultSimulationFilterShader;
		gScene = gPhysics->createScene(sceneDesc);

#ifdef _DEBUG
		PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
		if (pvdClient)
		{
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}
#endif // _DEBUG

		// Registering Resources
		PhysicsMaterialResource::Register();

		// Registering Components
		BoxColliderComponent::StaticConstructor();
		SphereColliderComponent::StaticConstructor();
		CapsuleColliderComponent::StaticConstructor();
		RigidbodyComponent::StaticConstructor();

		// Create default resources
		gDefaultMaterial = D_RESOURCE::GetManager()->CreateResource<PhysicsMaterialResource>(D_CORE::GenerateUuidFor("Default Physics Material"), L"Default Physics Material", L"Default Physics Material", true);
	}

	void Shutdown()
	{
		D_ASSERT(_init);

		PX_RELEASE(gScene);
		PX_RELEASE(gDispatcher);
		PX_RELEASE(gPhysics);

		if (gPvd)
		{
			auto transport = gPvd->getTransport();
			if (gPvd)
				gPvd->release();
			gPvd = nullptr;
			PX_RELEASE(transport);
		}

		PX_RELEASE(gFoundation);
	}

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options)
	{
		return false;
	}
#endif

	void Update(bool running)
	{
		D_PROFILING::ScopedTimer physicsProfiler(L"Physics Update");

		UpdatePrePhysicsTransform(running);

		if (running)
		{
			{
				D_PROFILING::ScopedTimer physicsProfiler(L"Physics Simulation Update");
				gScene->simulate(D_TIME::GetTargetElapsedSeconds());
				gScene->fetchResults(true);
			}

			UpdatePostPhysicsTransforms();
		}
	}

	void UpdatePostPhysicsTransforms()
	{
		D_PROFILING::ScopedTimer physicsProfiler(L"Physics Post Update");

		UINT numRigidBodyComps = D_WORLD::CountComponents<RigidbodyComponent>();
		if (numRigidBodyComps <= 0)
			return;

		D_CONTAINERS::DVector<std::function<void()>> updateFuncs;
		updateFuncs.reserve(numRigidBodyComps);

		D_WORLD::IterateComponents<RigidbodyComponent>([&](RigidbodyComponent& comp)
			{
				updateFuncs.push_back([&]()
					{
						if (comp.IsActive())
							comp.Update(-1);

					});
			}
		);

		D_JOB::AddTaskSetAndWait(updateFuncs);

	}

	void UpdatePrePhysicsTransform(bool simulating)
	{
		D_PROFILING::ScopedTimer physicsProfiler(L"Physics Post Update");

		D_WORLD::IterateComponents<BoxColliderComponent>([&](BoxColliderComponent& colliderComp)
			{
				if (colliderComp.IsActive())
					colliderComp.PreUpdate(simulating);
			}
		);

		D_WORLD::IterateComponents<SphereColliderComponent>([&](SphereColliderComponent& colliderComp)
			{
				if (colliderComp.IsActive())
					colliderComp.PreUpdate(simulating);
			}
		);

		D_WORLD::IterateComponents<CapsuleColliderComponent>([&](CapsuleColliderComponent& colliderComp)
			{
				if (colliderComp.IsActive())
					colliderComp.PreUpdate(simulating);
			}
		);

		D_WORLD::IterateComponents<RigidbodyComponent>([&](RigidbodyComponent& rigidbodyComp)
			{
				if (rigidbodyComp.IsActive())
					rigidbodyComp.PreUpdate();
			}
		);
	}

	PxScene* GetScene()
	{
		return gScene;
	}

	PxPhysics* GetCore()
	{
		return gPhysics;
	}

	D_RESOURCE::ResourceHandle GetDefaultMaterial()
	{
		return gDefaultMaterial;
	}

	bool CastRay(_IN_ D_MATH::Vector3 const& origin, _IN_ D_MATH::Vector3 const& direction, _IN_ float maxDistance, _OUT_ physx::PxRaycastBuffer& hit)
	{
		return gScene->raycast(*(PxVec3 const*)&origin, *(PxVec3 const*)&direction, maxDistance, hit);
	}

	bool CastCapsule(_IN_ D_MATH::Vector3 const& origin, _IN_ D_MATH::Vector3 const& direction, float radius, float halfHeight, D_MATH::Quaternion const& capsuleRotation, float maxDistance, physx::PxSweepBuffer& hit)
	{
		D_ASSERT(radius > 0);
		D_ASSERT(halfHeight > 0);
		return gScene->sweep(PxCapsuleGeometry(radius, halfHeight), PxTransform(GetVec3(origin), GetQuat(capsuleRotation)), GetVec3(direction.Normalize()), maxDistance, hit);
	}

	bool CastSphere(D_MATH::Vector3 const& origin, D_MATH::Vector3 const& direction, float radius, float maxDistance, _OUT_ physx::PxSweepBuffer& hit)
	{
		D_ASSERT(radius > 0);
		return gScene->sweep(PxSphereGeometry(radius), PxTransform(GetVec3(origin)), GetVec3(direction.Normalize()), maxDistance, hit);
	}

	bool CastBox(D_MATH::Vector3 const& origin, D_MATH::Vector3 const& direction, D_MATH::Vector3 const& halfExtents, D_MATH::Quaternion const& boxRotation, float maxDistance, _OUT_ physx::PxSweepBuffer& hit)
	{
		D_ASSERT(DirectX::XMVector3Greater(halfExtents, DirectX::XMVectorZero()));

		return gScene->sweep(PxBoxGeometry(GetVec3(halfExtents)), PxTransform(GetVec3(origin), GetQuat(boxRotation)), GetVec3(direction.Normalize()), maxDistance, hit);
	}

	bool CastRay_DebugDraw(_IN_ D_MATH::Vector3 const& origin, _IN_ D_MATH::Vector3 const& direction, _IN_ float maxDistance, float secendsToDisplay, _OUT_ physx::PxRaycastBuffer& hit)
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

	bool CastCapsule_DebugDraw(_IN_ D_MATH::Vector3 const& origin, _IN_ D_MATH::Vector3 const& direction, float maxDistance, float radius, float halfHeight, D_MATH::Quaternion const& capsuleRotation, float secendsToDisplay, _OUT_ physx::PxSweepBuffer& hit)
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

	bool CastSphere_DebugDraw(D_MATH::Vector3 const& origin, D_MATH::Vector3 const& direction, float radius, float maxDistance, float secendsToDisplay, _OUT_ physx::PxSweepBuffer& hit)
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

	bool CastBox_DebugDraw(D_MATH::Vector3 const& origin, D_MATH::Vector3 const& direction, D_MATH::Vector3 const& halfExtents, D_MATH::Quaternion const& boxRotation, float maxDistance, float secendsToDisplay, _OUT_ physx::PxSweepBuffer& hit)
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
