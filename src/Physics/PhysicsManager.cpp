#include "pch.hpp"
#include "PhysicsManager.hpp"

#include "Components/RigidbodyComponent.hpp"
#include "Components/BoxColliderComponent.hpp"
#include "Components/SphereColliderComponent.hpp"
#include "Components/CapsuleColliderComponent.hpp"
#include "Components/MeshColliderComponent.hpp"
#include "Resources/PhysicsMaterialResource.hpp"

#include <Core/Containers/Map.hpp>
#include <Core/TimeManager/TimeManager.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>
#include <Job/Job.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Scene/Scene.hpp>
#include <Utils/Assert.hpp>

#include <PxPhysics.h>
#include <cooking/PxCooking.h>

#if _D_EDITOR
#include <imgui.h>
#endif

#define PX_RELEASE(x)	if(x)	{ x->release(); x = NULL;	}

using namespace D_CORE;
using namespace D_CONTAINERS;
using namespace physx;

DUnorderedMap<Uuid, std::pair<UINT, PxConvexMesh*>, UuidHasher>		ConvexMeshCache;
std::mutex												CacheAccessMutex;

namespace Darius::Physics
{
	bool									_init = false;

	bool									dirtyOptions = false;

	PxDefaultAllocator						gAllocator;
	PxDefaultErrorCallback					gErrorCallback;
	PxTolerancesScale						gToleranceScale;

	PxFoundation* gFoundation = NULL;
	PxPhysics* gPhysics = NULL;
	PxCooking* gCooking = NULL;

	PxDefaultCpuDispatcher* gDispatcher = NULL;
	std::unique_ptr<PhysicsScene> gScene;

	PxPvd* gPvd = NULL;

	D_RESOURCE::ResourceHandle				gDefaultMaterial;

	void					UpdatePostPhysicsTransforms();
	void					UpdatePrePhysicsTransform(bool simulating);

	void Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(!_init);
		_init = true;

		// Initializing options
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("Physics.Tolerance.Length", gToleranceScale.length, 1.f);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("Physics.Tolerance.Speed", gToleranceScale.speed, 10.f);

		// Initializing Physic Foundation
		gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
		D_ASSERT_M(gFoundation, "Could not initialize physic foundation.");

#ifdef _DEBUG
		gPvd = PxCreatePvd(*gFoundation);
		PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
#endif

		// Initializing Physics Core
		gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, gToleranceScale, true, gPvd);
		D_ASSERT_M(gPhysics, "Could not initialize physics core.");

		gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(gToleranceScale));

		PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
		sceneDesc.gravity = PxVec3(0.f, -9.8f, 0.f);
		gDispatcher = PxDefaultCpuDispatcherCreate(2);
		sceneDesc.cpuDispatcher = gDispatcher;
		sceneDesc.filterShader = PxDefaultSimulationFilterShader;
		gScene = std::make_unique<PhysicsScene>(sceneDesc, gPhysics);

		// Registering Resources
		PhysicsMaterialResource::Register();

		// Registering Components
		BoxColliderComponent::StaticConstructor();
		SphereColliderComponent::StaticConstructor();
		CapsuleColliderComponent::StaticConstructor();
		MeshColliderComponent::StaticConstructor();
		RigidbodyComponent::StaticConstructor();

		// Create default resources
		gDefaultMaterial = D_RESOURCE::GetManager()->CreateResource<PhysicsMaterialResource>(D_CORE::GenerateUuidFor("Default Physics Material"), L"Default Physics Material", L"Default Physics Material", true);
	}

	void Shutdown()
	{
		D_ASSERT(_init);

		gScene.reset();
		PX_RELEASE(gDispatcher);
		PX_RELEASE(gCooking);
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
		D_H_OPTION_DRAW_BEGIN();

		if (dirtyOptions)
		{
			ImGui::TextColored({ 1.f, 1.f, 0.f, 1.f }, "You have to restart the engine for Physics options to take effect!");
		}

		ImGui::Text("Tolerance");
		ImGui::Separator();
		{
			D_H_OPTION_DRAW_FLOAT_SLIDER_EXP("Speed Tolerance", "Physics.Tolerance.Speed", gToleranceScale.speed, 1.f, 100.f);
			D_H_OPTION_DRAW_FLOAT_SLIDER_EXP("Length Tolerance", "Physics.Tolerance.Length", gToleranceScale.length, 0.01f, 100.f);
		}
		ImGui::Spacing();

		dirtyOptions |= settingsChanged;

		D_H_OPTION_DRAW_END();
	}
#endif

	void Flush()
	{
		gScene->Simulate(true, D_TIME::GetTargetElapsedSeconds());
	}

	void Update(bool running)
	{
		D_PROFILING::ScopedTimer physicsProfiler(L"Physics Update");

		UpdatePrePhysicsTransform(running);

		if (running)
		{
			gScene->Simulate(true, D_TIME::GetTargetElapsedSeconds());

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

		D_WORLD::IterateComponents<MeshColliderComponent>([&](MeshColliderComponent& colliderComp)
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

	PhysicsScene* GetScene()
	{
		return gScene.get();
	}

	PxPhysics* GetCore()
	{
		return gPhysics;
	}

	D_RESOURCE::ResourceHandle GetDefaultMaterial()
	{
		return gDefaultMaterial;
	}

	PxConvexMesh* CreateConvexMesh(D_CORE::Uuid const& uuid, bool direct, physx::PxConvexMeshDesc const& desc)
	{
		D_ASSERT(desc.flags & PxConvexFlag::eCOMPUTE_CONVEX);

		// Lookup the cache to retreive the mesh if already created for the given uuid
		{
			std::scoped_lock accessLock(CacheAccessMutex);
			if (ConvexMeshCache.contains(uuid))
			{
				auto& pair = ConvexMeshCache.at(uuid);
				pair.first++;
				return pair.second;
			}
		}
		
		PxConvexMesh* convex;

		// Creating convex mesh
		if (direct)
		{
#ifdef _DEBUG
			if (desc.flags & PxConvexFlag::eDISABLE_MESH_VALIDATION)
			{
				// mesh should be validated before cooking without the mesh cleaning
				bool res = gCooking->validateConvexMesh(desc);
				D_ASSERT(res);
			}
#endif
			convex = gCooking->createConvexMesh(desc, gPhysics->getPhysicsInsertionCallback());
		}
		else
		{
			PxDefaultMemoryOutputStream buf;
			PxConvexMeshCookingResult::Enum result;
			bool test = gCooking->cookConvexMesh(desc, buf, &result);
			D_ASSERT(test);

			PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
			convex = gPhysics->createConvexMesh(input);
		}

		// Adding the created mesh to the cache
		{
			std::scoped_lock accessLock(CacheAccessMutex);
			auto& pair = ConvexMeshCache[uuid];
			pair.first = 1u;
			pair.second = convex;
		}

		return convex;
	}

	void ReleaseConvexMesh(D_CORE::Uuid const& uuid)
	{
		std::scoped_lock accessLock(CacheAccessMutex);
		if (!ConvexMeshCache.contains(uuid))
			return;

		auto& pair = ConvexMeshCache.at(uuid);
		pair.first--;

		if (pair.first <= 0u)
		{
			pair.second->release();
			ConvexMeshCache.erase(uuid);
		}
	}

}
