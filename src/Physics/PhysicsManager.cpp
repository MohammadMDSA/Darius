#include "pch.hpp"
#include "PhysicsManager.hpp"

#include "Components/ColliderComponent.hpp"
#include "Components/BoxColliderComponent.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Scene/Scene.hpp>
#include <Renderer/GraphicsUtils/Profiling/Profiling.hpp>
#include <Job/Job.hpp>
#include <Utils/Assert.hpp>

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

	PxMaterial* gDefaultMaterial = NULL;

	PxPvd* gPvd = NULL;

	void					UpdatePostPhysicsTransforms();
	void					UpdatePrePhysicsTransform(bool simulating);

	void Initialize()
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

		gDefaultMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.f);

		ColliderComponent::StaticConstructor();
		BoxColliderComponent::StaticConstructor();
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
			gPvd->release();
			gPvd = nullptr;
			PX_RELEASE(transport);
		}

		PX_RELEASE(gFoundation);
	}

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
		D_WORLD::GetRegistry().each([&](BoxColliderComponent& colliderComp)
			{
				D_JOB::AssignTask([&](int threadNumber, int)
					{
						colliderComp.Update(-1);

					});
			}
		);

		if (D_JOB::IsMainThread())
			D_JOB::WaitForThreadsToFinish();
	}

	void UpdatePrePhysicsTransform(bool simulating)
	{
		D_PROFILING::ScopedTimer physicsProfiler(L"Physics Post Update");
		D_WORLD::GetRegistry().each([&](BoxColliderComponent& colliderComp)
			{
				colliderComp.PreUpdate(simulating);
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

	PxMaterial const* GetDefaultMaterial()
	{
		return gDefaultMaterial;
	}

}
