#include "Renderer/pch.hpp"
#include "RayTracingScene.hpp"

#include "RayTracingCommandContext.hpp"

#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>

using namespace D_GRAPHICS_UTILS;
using namespace D_RENDERER_RT_UTILS;
using namespace D_PROFILING;

namespace Darius::Renderer::RayTracing
{
	RayTracingScene::RayTracingScene(UINT initialNumBLAS) :
		mTLASSizeDirty(true)
	{
		D_ASSERT(initialNumBLAS > 0);

		mBottomLevelASInstanceDescs.Create(L"Bottom-Level Acceleration Structure Instance descs", initialNumBLAS, D_GRAPHICS_DEVICE::gNumFrameResources);
		mInstancesUuid.resize(initialNumBLAS);

		InitializeTopLevelAS(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE, false, false, L"Top Level Acceleration Structure");
	}

	RayTracingScene::~RayTracingScene()
	{
		Reset();
	}

	// Adds a bottom-level Acceleration Structure.
	// The passed in bottom-level AS geometry must have a unique name.
	// Requires a corresponding 1 or more AddBottomLevelASInstance() calls to be added to the top-level AS for the bottom-level AS to be included.
	void RayTracingScene::AddBottomLevelAS(
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags,
		BottomLevelAccelerationStructureGeometry const& bottomLevelASGeometry,
		bool allowUpdate,
		bool performUpdateOnBuild)
	{
		D_ASSERT_M(mVBottomLevelAS.find(bottomLevelASGeometry.Uuid) == mVBottomLevelAS.end(),
			L"A bottom level acceleration structure with that name already exists.");

		auto& bottomLevelAS = mVBottomLevelAS[bottomLevelASGeometry.Uuid];

		bottomLevelAS.Initialize(buildFlags, bottomLevelASGeometry, allowUpdate);

		mASmemoryFootprint += bottomLevelAS.RequiredResultDataSizeInBytes();
		mScratchResourceSize = std::max(bottomLevelAS.RequiredScratchSize(), mScratchResourceSize);

		mVBottomLevelAS[bottomLevelAS.GetUuid()] = bottomLevelAS;
	}

	// Adds an instance of a bottom-level Acceleration Structure.
	// Requires a call InitializeTopLevelAS() call to be added to top-level AS.
	UINT RayTracingScene::AddBottomLevelASInstance(
		D_CORE::Uuid const& bottomLevelASUuid,
		UINT instanceContributionToHitGroupIndex,
		D_MATH::Matrix4 const& transform,
		BYTE instanceMask)
	{
		UINT instanceIndex = mNumBottomLevelASInstances++;

		if (mNumBottomLevelASInstances > mBottomLevelASInstanceDescs.GetNumElements())
		{
			// Resizing to next power of two to avoid frequent resizes
			auto newSize = D_MATH::RoundUpToPowerOfTwo(instanceIndex);
			mBottomLevelASInstanceDescs.Create(L"Bottom - Level Acceleration Structure Instance descs", newSize, D_GRAPHICS_DEVICE::gNumFrameResources);
			mInstancesUuid.resize(newSize);
			mTLASSizeDirty = true;
		}

		auto& bottomLevelAS = mVBottomLevelAS[bottomLevelASUuid];

		auto& instanceDesc = mBottomLevelASInstanceDescs[instanceIndex];
		mInstancesUuid[instanceIndex] = bottomLevelASUuid;

		instanceDesc.InstanceMask = instanceMask;
		instanceDesc.InstanceContributionToHitGroupIndex = instanceContributionToHitGroupIndex != UINT_MAX ? instanceContributionToHitGroupIndex : bottomLevelAS.GetInstanceContributionToHitGroupIndex();
		instanceDesc.AccelerationStructure = bottomLevelAS.GetResource()->GetGPUVirtualAddress();
		XMStoreFloat3x4(reinterpret_cast<DirectX::XMFLOAT3X4*>(instanceDesc.Transform), transform);

		return instanceIndex;
	};

	UINT RayTracingScene::GetMaxInstanceContributionToHitGroupIndex() const
	{
		UINT maxInstanceContributionToHitGroupIndex = 0;
		for (UINT i = 0; i < mNumBottomLevelASInstances; i++)
		{
			auto& instanceDesc = mBottomLevelASInstanceDescs[i];
			maxInstanceContributionToHitGroupIndex = std::max(maxInstanceContributionToHitGroupIndex, instanceDesc.InstanceContributionToHitGroupIndex);
		}
		return maxInstanceContributionToHitGroupIndex;
	};

	// Initializes the top-level Acceleration Structure.
	void RayTracingScene::InitializeTopLevelAS(
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags,
		bool allowUpdate,
		bool performUpdateOnBuild,
		const wchar_t* resourceName)
	{
		// Initialize with current max number of BLASes instances
		mTLAS.Initialize(mBottomLevelASInstanceDescs.GetNumElements(), buildFlags, allowUpdate, performUpdateOnBuild, resourceName);

		mASmemoryFootprint += mTLAS.RequiredResultDataSizeInBytes();
		mScratchResourceSize = std::max(mTLAS.RequiredScratchSize(), mScratchResourceSize);

		mAccelerationStructureScratch.Create(L"Acceleration structure scratch resource", 1, (UINT32)mScratchResourceSize);
		mTLASSizeDirty = false;
	}

	ShaderTable* RayTracingScene::FindExistingShaderTable(RayTracingStateObject const* stateObject) const
	{
		auto foundShaderTable = mShaderTables.find(stateObject);

		if (foundShaderTable == mShaderTables.end())
			return nullptr;

		return foundShaderTable->second;
	}

	ShaderTable* RayTracingScene::FindOrCreateShaderTable(RayTracingStateObject const* stateObject)
	{
		D_PROFILING::ScopedTimer _prof(L"FindOrCreateShaderTable");

		ShaderTable* createdShaderTable = new ShaderTable();
		auto device = D_GRAPHICS_DEVICE::GetDevice5();

		const UINT numHitGroupSlots = mNumTotalSegments * mNumShaderSlotPerGeometrySegment;

		ShaderTable::Initializer initializer = {};
		initializer.NumRayGenShaders = stateObject->GetRayGenerationShaders().size();
		initializer.NumMissShaders = stateObject->GetMissShaders().size();
		initializer.NumMissRecords = NumMissShaderSlots;
		initializer.NumHitRecords = numHitGroupSlots;
		initializer.NumCallableRecords = NumCallableShaderSlots;
		initializer.LocalRootDataSize = stateObject->GetMaxLocalRootSignatureSize();
		
		D_CONTAINERS::DVector<Shaders::ShaderIdentifier> rayGenIdentifiers;

		auto const& rayGenShaders = stateObject->GetRayGenerationShaders();

		std::transform(rayGenShaders.begin(), rayGenShaders.end(), std::back_inserter(rayGenIdentifiers), [](auto& rayGenShader) { return rayGenShader->Identifier; });

		createdShaderTable->Init(initializer, device, rayGenIdentifiers, stateObject->GetHitGroups().front().Identifier);
		
		mShaderTables[stateObject] = createdShaderTable;

		return createdShaderTable;
	}


	// Builds all bottom-level and top-level Acceleration Structures.
	void RayTracingScene::Build(
		UINT numShaderSlotPerGeometry,
		RayTracingCommandContext& commandList,
		ID3D12DescriptorHeap* descriptorHeap,
		UINT frameIndex,
		bool bForceBuild)
	{
		ScopedTimer _prof(L"Acceleration Structure build", commandList);
		
		{
			mNumShaderSlotPerGeometrySegment = numShaderSlotPerGeometry;
			mNumTotalSegments = 0u;
			for (auto const& instanceUuid : mInstancesUuid)
			{
				mNumTotalSegments += mVBottomLevelAS[instanceUuid].GetNumGeometries();
			}
		}

		mBottomLevelASInstanceDescs.CopyStagingToGpu(frameIndex);

		// Build all bottom-level AS.
		{
			ScopedTimer _prof(L"Bottom Level AS", commandList);
			for (auto& bottomLevelASpair : mVBottomLevelAS)
			{
				auto& bottomLevelAS = bottomLevelASpair.second;
				if (bForceBuild || bottomLevelAS.IsDirty())
				{
					ScopedTimer _prof(bottomLevelAS.GetName(), commandList);

					D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGpuAddress = 0;
					bottomLevelAS.Build(commandList, mAccelerationStructureScratch, baseGeometryTransformGpuAddress);

					// Since a single scratch resource is reused, put a barrier in-between each call.
					// PEFORMANCE tip: use separate scratch memory per BLAS build to allow a GPU driver to overlap build calls.
					commandList.TransitionResource(bottomLevelAS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				}
			}
		}

		// Initialize TLAS again if necessary
		if (mTLASSizeDirty)
		{
			InitializeTopLevelAS(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE, false, false, L"Top Level Acceleration Structure");
		}

		// Build the top-level AS.
		{
			ScopedTimer _prof(L"Top Level AS", commandList);

			bool performUpdate = false; // Always rebuild top-level Acceleration Structure.
			mTLAS.Build(commandList, GetNumberOfBottomLevelASInstances(), mBottomLevelASInstanceDescs, frameIndex, mAccelerationStructureScratch, performUpdate);

			commandList.TransitionResource(mTLAS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
		}
	}

	void RayTracingScene::Reset()
	{
		NumMissShaderSlots = 1;
		NumCallableShaderSlots = 0;
		mNumBottomLevelASInstances = 0;

		for (auto& table : mShaderTables)
			delete table.second;

		mShaderTables.clear();
	}
}