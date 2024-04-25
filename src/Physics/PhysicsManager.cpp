#include "pch.hpp"
#include "PhysicsManager.hpp"

#include "Components/RigidbodyComponent.hpp"
#include "Components/BoxColliderComponent.hpp"
#include "Components/SphereColliderComponent.hpp"
#include "Components/CapsuleColliderComponent.hpp"
#include "Components/MeshColliderComponent.hpp"
#include "Components/CharacterControllerComponent.hpp"
#include "Resources/PhysicsMaterialResource.hpp"

#include <Core/Containers/Map.hpp>
#include <Core/Containers/Vector.hpp>
#include <Core/TimeManager/TimeManager.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>
#include <Job/Job.hpp>
#include <Renderer/Resources/StaticMeshResource.hpp>
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
using namespace D_RENDERER_GEOMETRY;
using namespace physx;


namespace Darius::Physics
{
	bool									_init = false;

	bool									dirtyOptions = false;
	bool									gGpuAccelerated = false;

	PxDefaultAllocator						gAllocator;
	PxDefaultErrorCallback					gErrorCallback;
	PxTolerancesScale						gToleranceScale;
	PxCookingParams*						gCookingParams;

	PxFoundation*							gFoundation = NULL;
	PxPhysics*								gPhysics = NULL;
	PxCudaContextManager*					gCudaContextManager = NULL;

	PxDefaultCpuDispatcher* gDispatcher = NULL;
	std::unique_ptr<PhysicsScene> gScene;

	PxPvd* gPvd = NULL;

	D_RESOURCE::ResourceHandle							gDefaultMaterial;

	DUnorderedMap<Uuid, ConvexMeshData, UuidHasher>		ConvexMeshDataTable;
	DUnorderedMap<Uuid, TriangleMeshData, UuidHasher>	TriangleMeshDataTable;

	std::mutex											ConvexMeshTableAccessMutex;
	std::mutex											TriangleMeshTableAccessMutex;

	void					UpdatePostPhysicsTransforms();
	void					UpdatePrePhysicsTransform(bool simulating);

	struct AsyncConvexMeshCreationTask : public D_JOB::IPinnedTask
	{
		AsyncConvexMeshCreationTask(physx::PxCookingParams const& cooking) :
			cookingParams(cooking) {}


		virtual void Execute() override
		{
			if (cancellationToken && cancellationToken->IsCancelled())
				return;

			auto handle = CreateConvexMesh(uuid, direct, desc);

			if (callback)
				callback(handle);
		}

		~AsyncConvexMeshCreationTask()
		{
			D_LOG_DEBUG("AsyncConvexMeshCreationTask destruction");
		}

		D_JOB::CancellationToken* cancellationToken = nullptr;
		bool direct = false;
		Uuid uuid;
		physx::PxConvexMeshDesc desc;
		physx::PxCookingParams cookingParams;
		MeshCreationCallback callback;
	};

	struct AsyncTriangleMeshCreationTask : public D_JOB::IPinnedTask
	{

		virtual void Execute() override
		{
			if (cancellationToken && cancellationToken->IsCancelled())
				return;

			auto handle = CreateTriangleMesh(uuid, direct, desc, skipMeshCleanup, skipEdgeData, numTrisPerLeaf);

			if (callback)
				callback(handle);
		}

		~AsyncTriangleMeshCreationTask()
		{
			D_LOG_DEBUG("AsyncTriangleMeshCreationTask destruction");
		}

		D_JOB::CancellationToken* cancellationToken = nullptr;
		Uuid uuid;
		UINT numTrisPerLeaf;
		physx::PxTriangleMeshDesc desc;
		bool direct = false;
		bool skipMeshCleanup = false;
		bool skipEdgeData = false;
		MeshCreationCallback callback;
	};

	void Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(!_init);
		_init = true;

		// Initializing options
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("Physics.GpuAccelerated", gGpuAccelerated, true);
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

		// Initializing Cuda
		if(gGpuAccelerated)
		{
			PxCudaContextManagerDesc cudaContextManagerDesc;
			cudaContextManagerDesc.graphicsDevice = D_GRAPHICS_DEVICE::GetDevice();
			gCudaContextManager = PxCreateCudaContextManager(*gFoundation, cudaContextManagerDesc, PxGetProfilerCallback());	//Create the CUDA context manager, required for GRB to dispatch CUDA kernels.
			if(gCudaContextManager)
			{
				if(!gCudaContextManager->contextIsValid())
					PX_RELEASE(gCudaContextManager);
			}
		}

		PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
		sceneDesc.gravity = PxVec3(0.f, -9.8f, 0.f);
		gDispatcher = PxDefaultCpuDispatcherCreate(std::thread::hardware_concurrency());
		sceneDesc.cpuDispatcher = gDispatcher;
		sceneDesc.filterShader = PxDefaultSimulationFilterShader;
		if(gGpuAccelerated && gCudaContextManager)
		{
			sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
			sceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;
			sceneDesc.cudaContextManager = gCudaContextManager;
		}


		// Cooking params
		gCookingParams = new PxCookingParams(gToleranceScale);
		gCookingParams->buildGPUData = gGpuAccelerated;
		gCookingParams->midphaseDesc = PxMeshMidPhase::eBVH34;

		gScene = std::make_unique<PhysicsScene>(sceneDesc, gPhysics);

		// Registering Resources
		PhysicsMaterialResource::Register();

		// Registering Components
		BoxColliderComponent::StaticConstructor();
		SphereColliderComponent::StaticConstructor();
		CapsuleColliderComponent::StaticConstructor();
		MeshColliderComponent::StaticConstructor();
		RigidbodyComponent::StaticConstructor();
		CharacterControllerComponent::StaticConstructor();

		// Create default resources
		gDefaultMaterial = D_RESOURCE::GetManager()->CreateResource<PhysicsMaterialResource>(D_CORE::GenerateUuidFor("Default Physics Material"), L"Default Physics Material", L"Default Physics Material", true);
	}

	void Shutdown()
	{
		D_ASSERT(_init);

		gScene.reset();

		delete gCookingParams;

		PX_RELEASE(gCudaContextManager);
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
		D_H_OPTION_DRAW_BEGIN();

		if (dirtyOptions)
		{
			ImGui::TextColored({ 1.f, 1.f, 0.f, 1.f }, "You have to restart the engine for Physics options to take effect!");
		}

		// Gpu Accelerated
		{
			D_H_OPTION_DRAW_CHECKBOX("Gpu Accelerated", "Physics.GpuAccelerated", gGpuAccelerated);
		}

		ImGui::Spacing();
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

	std::string GetConvexMeshCookingResultText(PxConvexMeshCookingResult::Enum value)
	{
		switch (value)
		{
		case physx::PxConvexMeshCookingResult::eSUCCESS:
			return "Success";
		case physx::PxConvexMeshCookingResult::eZERO_AREA_TEST_FAILED:
			return "Zero Area Test Failed";
		case physx::PxConvexMeshCookingResult::ePOLYGONS_LIMIT_REACHED:
			return "Polygons Limit Reached";
		case physx::PxConvexMeshCookingResult::eFAILURE:
			return "Failure";
		case physx::PxConvexMeshCookingResult::eNON_GPU_COMPATIBLE:
			return "Non Gpu Compatible";
		default:
			return "";
		}
	}

	std::string GetTriangleMeshCookingResultText(PxTriangleMeshCookingResult::Enum value)
	{
		switch (value)
		{
		case physx::PxTriangleMeshCookingResult::eSUCCESS:
			return "Success";
		case physx::PxTriangleMeshCookingResult::eLARGE_TRIANGLE:
			return "Large Triangle";
		case physx::PxTriangleMeshCookingResult::eEMPTY_MESH:
			return "Empty Mesh";
		case physx::PxTriangleMeshCookingResult::eFAILURE:
			return "Failure";
		default:
			return "";
		}
	}

	void CreateConvexMeshAsync(D_CORE::Uuid const& uuid, bool direct, physx::PxConvexMeshDesc const& desc, MeshCreationCallback callback, Darius::Job::CancellationToken* cancelleationToken)
	{
		auto task = new AsyncConvexMeshCreationTask(*gCookingParams);
		task->callback = callback;
		task->cancellationToken = cancelleationToken;
		task->desc = desc;
		task->direct = direct;
		task->uuid = uuid;

		D_JOB::AddPinnedTask(task, D_JOB::ThreadType::FileIO);
	}

	void CreateTriangleMeshAsync(D_CORE::Uuid const& uuid, bool direct, physx::PxTriangleMeshDesc const& desc, bool skipMeshCleanup, bool skipEdgeData, UINT numTrisPerLeaf, MeshCreationCallback callback, D_JOB::CancellationToken* cancelleationToken)
	{
		auto task = new AsyncTriangleMeshCreationTask();
		task->callback = callback;
		task->cancellationToken = cancelleationToken;
		task->desc = desc;
		task->direct = direct;
		task->uuid = uuid;
		task->skipEdgeData = skipMeshCleanup;
		task->skipEdgeData = skipEdgeData;
		task->numTrisPerLeaf = numTrisPerLeaf;
		
		D_JOB::AddPinnedTask(task, D_JOB::ThreadType::FileIO);
	}

	MeshDataHandle CreateTriangleMesh(D_CORE::Uuid const& uuid, bool direct, physx::PxTriangleMeshDesc const& desc,
		bool skipMeshCleanup, bool skipEdgeData, UINT numTrisPerLeaf)
	{
		// Lookup the cache to retreive the mesh if already created for the given uuid
		{
			std::scoped_lock accessLock(TriangleMeshTableAccessMutex);
			if (TriangleMeshDataTable.contains(uuid))
			{
				auto& data = TriangleMeshDataTable.at(uuid);
				return &data;
			}
		}

		PxCookingParams cookParams(*gCookingParams);

		// Setting up cooking params
		if (skipMeshCleanup)
			cookParams.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
		else
			cookParams.meshPreprocessParams &= ~static_cast<PxMeshPreprocessingFlags>(PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH);

		if (!skipEdgeData)
			cookParams.meshPreprocessParams &= ~static_cast<PxMeshPreprocessingFlags>(PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE);
		else
			cookParams.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;

		cookParams.midphaseDesc.mBVH34Desc.numPrimsPerLeaf = numTrisPerLeaf;

		PxTriangleMesh* triMesh = nullptr;
		PxTriangleMeshCookingResult::Enum condition;

		if (direct)
		{
#if _DEBUG
			if (skipMeshCleanup)
			{
				D_ASSERT(PxValidateTriangleMesh(cookParams, desc));
			}
#endif // _DEBUG

			triMesh = PxCreateTriangleMesh(cookParams, desc, gPhysics->getPhysicsInsertionCallback(), &condition);

			if (!D_VERIFY(triMesh))
			{
				auto log = std::format("Cooking triangle mesh failed with result: {}", GetTriangleMeshCookingResultText(condition));
				D_LOG_WARN(log);
			}
		}
		else
		{
			PxDefaultMemoryOutputStream outBuffer;
			bool test = PxCookTriangleMesh(cookParams, desc, outBuffer, &condition);

			if (!D_VERIFY(test))
			{
				D_LOG_WARN(std::format("Cooking convex mesh failed with result: {}", GetTriangleMeshCookingResultText(condition)));
			}

			PxDefaultMemoryInputData stream(outBuffer.getData(), outBuffer.getSize());
			triMesh = gPhysics->createTriangleMesh(stream);
		}

		// Adding the created mesh to the cache
		{
			std::scoped_lock accessLock(TriangleMeshTableAccessMutex);
			TriangleMeshDataTable.emplace(uuid, TriangleMeshData(triMesh, uuid));
			auto& meshData = TriangleMeshDataTable.at(uuid);

#if _D_EDITOR
			// Creating debug mesh
			{
				auto& mesh = meshData.Mesh;
				mesh.Name = L"Physics Convex Mesh Debug";

				// Loading vertices buffer
				{
					// Loading positions only
					DVector<D_RENDERER::StaticMeshResource::VertexType, boost::alignment::aligned_allocator<D_RENDERER::StaticMeshResource::VertexType, 16>> vertices;
					vertices.resize(triMesh->getNbVertices());
					auto pxVerts = triMesh->getVertices();
					for (UINT i = 0; i < triMesh->getNbVertices(); i++)
					{
						vertices[i].mPosition = DirectX::XMFLOAT3((D_PHYSICS::GetVec3(pxVerts[i])));
					}
					// Create vertex buffer
					mesh.VertexDataGpu.Create(L"Physics Triangle Mesh Debug Vertices", triMesh->getNbVertices(), sizeof(D_RENDERER::StaticMeshResource::VertexType), vertices.data());
					mesh.mNumTotalVertices = triMesh->getNbVertices();
				}

				// Loading indices
				{
					DVector<UINT, boost::alignment::aligned_allocator<UINT, 16>> indices;
					auto indexBuff = triMesh->getTriangles();

					indices.resize(triMesh->getNbTriangles() * 3);
					auto src = indices.data();

					if (triMesh->getTriangleMeshFlags() & PxTriangleMeshFlag::e16_BIT_INDICES)
					{
						auto _16BitIndexBuff = reinterpret_cast<uint16_t const*>(indexBuff);

						for (UINT i = 0; i < triMesh->getNbTriangles() * 3; i++)
						{
							indices[i] = static_cast<UINT>(_16BitIndexBuff[i]);
						}
					}
					else
					{
						auto _32BitIndexBuff = reinterpret_cast<uint16_t const*>(indexBuff);

						for (UINT i = 0; i < triMesh->getNbTriangles() * 3; i++)
						{
							indices[i] = _32BitIndexBuff[i];
						}
					}

					mesh.mNumTotalIndices = (UINT)indices.size();
					mesh.mDraw.push_back({ mesh.mNumTotalIndices, 0u, 0u });
					mesh.CreateIndexBuffers(indices.data());
				}
			}
#endif
			return &meshData;

		}
	}

	MeshDataHandle CreateConvexMesh(D_CORE::Uuid const& uuid, bool direct, physx::PxConvexMeshDesc const& desc)
	{
		D_ASSERT(desc.flags & PxConvexFlag::eCOMPUTE_CONVEX);

		// Lookup the cache to retreive the mesh if already created for the given uuid
		{
			std::scoped_lock accessLock(ConvexMeshTableAccessMutex);
			if (ConvexMeshDataTable.contains(uuid))
			{
				auto& data = ConvexMeshDataTable.at(uuid);
				return &data;
			}
		}

		PxConvexMesh* convex;

		PxConvexMeshCookingResult::Enum result;

		// Creating convex mesh
		if (direct)
		{
#ifdef _DEBUG
			if (desc.flags & PxConvexFlag::eDISABLE_MESH_VALIDATION)
			{
				// mesh should be validated before cooking without the mesh cleaning
				auto cookingParams = gCookingParams;
				cookingParams->meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
				bool res = PxValidateConvexMesh(*gCookingParams, desc);
				D_ASSERT(res);
			}
#endif
			convex = PxCreateConvexMesh(*gCookingParams, desc, gPhysics->getPhysicsInsertionCallback(), &result);

			if (!D_VERIFY(convex))
			{
				auto log = std::format("Cooking convex mesh failed with result: {}", GetConvexMeshCookingResultText(result));
				D_LOG_WARN(log);
			}
		}
		else
		{
			PxDefaultMemoryOutputStream buf;
			bool test = PxCookConvexMesh(*gCookingParams, desc, buf, &result);

			if (!D_VERIFY(test))
			{
				D_LOG_WARN(std::format("Cooking convex mesh failed with result: {}", GetConvexMeshCookingResultText(result)));
			}

			PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
			convex = gPhysics->createConvexMesh(input);
		}

		// Adding the created mesh to the cache
		{
			std::scoped_lock accessLock(ConvexMeshTableAccessMutex);
			ConvexMeshDataTable.emplace(uuid, ConvexMeshData(convex, uuid));
			auto& meshData = ConvexMeshDataTable.at(uuid);

#if _D_EDITOR
			// Creating debug mesh
			{
				auto& mesh = meshData.Mesh;
				mesh.Name = L"Physics Convex Mesh Debug";

				// Loading vertices buffer
				{
					// Loading positions only
					DVector<D_RENDERER::StaticMeshResource::VertexType, boost::alignment::aligned_allocator<D_RENDERER::StaticMeshResource::VertexType, 16>> vertices;
					vertices.resize(convex->getNbVertices());
					auto pxVerts = convex->getVertices();
					for (UINT i = 0; i < convex->getNbVertices(); i++)
					{
						vertices[i].mPosition = DirectX::XMFLOAT3((D_PHYSICS::GetVec3(pxVerts[i])));
					}
					// Create vertex buffer
					mesh.VertexDataGpu.Create(L"Physics Convex Mesh Debug Vertices", convex->getNbVertices(), sizeof(D_RENDERER::StaticMeshResource::VertexType), vertices.data());
					mesh.mNumTotalVertices = convex->getNbVertices();
				}

				// Loading indices
				{
					DVector<UINT, boost::alignment::aligned_allocator<UINT, 16>> indices;
					auto indexBuff = convex->getIndexBuffer();
					indices.reserve(convex->getNbVertices() * 3);
					auto src = indices.data();

					for (UINT i = 0; i < convex->getNbPolygons(); i++)
					{
						PxHullPolygon polyData;
						convex->getPolygonData(i, polyData);
						UINT triangleCount = polyData.mNbVerts - 2;

						UINT index0 = indexBuff[polyData.mIndexBase];

						for (UINT j = 0; j < triangleCount; j++)
						{
							indices.push_back(index0);
							indices.push_back(indexBuff[polyData.mIndexBase + j + 1]);
							indices.push_back(indexBuff[polyData.mIndexBase + j + 2]);
						}

					}

					mesh.mNumTotalIndices = (UINT)indices.size();
					mesh.mDraw.push_back({ mesh.mNumTotalIndices, 0u, 0u });
					mesh.CreateIndexBuffers(indices.data());
				}
			}
#endif
			return &meshData;

		}
	}

	MeshDataHandle FindTriangleMesh(D_CORE::Uuid const& uuid)
	{
		if (!TriangleMeshDataTable.contains(uuid))
			return InvalidMeshDataHandle;

		return &TriangleMeshDataTable.at(uuid);
	}

	MeshDataHandle FindConvexMesh(D_CORE::Uuid const& uuid)
	{
		if (!ConvexMeshDataTable.contains(uuid))
			return InvalidMeshDataHandle;

		return &ConvexMeshDataTable.at(uuid);
	}

	template<typename T>
	void RemoveMeshFromTable(T& table, D_CORE::Uuid const& uuid)
	{
		D_ASSERT(table.contains(uuid));

		auto& el = table.at(uuid);

		el.PxMesh->release();

		table.erase(uuid);
	}

	bool BaseMeshData::Release()
	{
		switch (Type)
		{
		case MeshType::ConvexMesh:
			RemoveMeshFromTable(ConvexMeshDataTable, Uuid);
			break;
		case MeshType::TriangleMesh:
			RemoveMeshFromTable(TriangleMeshDataTable, Uuid);
			break;
		default:
			break;
		}

		return true;
	}

}
