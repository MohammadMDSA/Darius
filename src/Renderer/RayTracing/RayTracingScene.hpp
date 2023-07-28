#pragma once

#include "Utils/AccelerationStructure.hpp"
#include "Utils/ShaderTable.hpp"

#include <Graphics/GraphicsUtils/StateObject.hpp>

#ifndef D_RENDERER_RT
#define D_RENDERER_RT Darius::Renderer::RayTracing
#endif // !D_RENDERER_RT_UTILS

namespace Darius::Renderer::RayTracing
{
	enum class RayTracingSceneState
	{
		Writable, // Scene is being built and can't be bound as SRV
		Readable, // Scene can be used in ray tracing commands (can be bound as SRV)
	};

	class RayTracingScene
	{
	public:
		RayTracingScene(UINT maxNumBLASes);
		~RayTracingScene();

		void                            AddBottomLevelAS(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, BottomLevelAccelerationStructureGeometry const& bottomLevelASGeometry, bool allowUpdate = false, bool performUpdateOnBuild = false);
		UINT                            AddBottomLevelASInstance(D_CORE::Uuid const& bottomLevelASUuid, UINT instanceContributionToHitGroupIndex = UINT_MAX, D_MATH::Matrix4 const& transform = D_MATH::Matrix4::Identity, BYTE InstanceMask = 1);
		void                            InitializeTopLevelAS(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, bool allowUpdate = false, bool performUpdateOnBuild = false, const wchar_t* resourceName = nullptr);

		// Call every frame to build acceleration structures
		void                            Build(UINT numShaderSlotPerGeometry, RayTracingCommandContext& commandList, ID3D12DescriptorHeap* descriptorHeap, UINT frameIndex, bool bForceBuild = false);
		INLINE BottomLevelAccelerationStructureInstanceDesc const& GetBottomLevelASInstance(UINT bottomLevelASinstanceIndex) const { return mBottomLevelASInstanceDescs[bottomLevelASinstanceIndex]; }

		// Shader Table Funcs
		D_RENDERER_RT_UTILS::ShaderTable* FindExistingShaderTable(D_GRAPHICS_UTILS::RayTracingStateObject const* stateObject) const;
		D_RENDERER_RT_UTILS::ShaderTable* FindOrCreateShaderTable(D_GRAPHICS_UTILS::RayTracingStateObject const* stateObject);
		D_GRAPHICS_BUFFERS::StructuredUploadBuffer<BottomLevelAccelerationStructureInstanceDesc> const& GetBottomLevelASInstancesBuffer() const { return mBottomLevelASInstanceDescs; }

		INLINE BottomLevelAccelerationStructure const& GetBottomLevelAS(D_CORE::Uuid const& uuid) const { return mVBottomLevelAS.at(uuid); }
		INLINE TopLevelAccelerationStructure const& GetTopLevelAS() const { return mTLAS; }
		INLINE UINT64                   GetASMemoryFootprint() const { return mASmemoryFootprint; }
		INLINE UINT                     GetNumberOfBottomLevelASInstances() const { return mNumBottomLevelASInstances; }
		UINT                            GetMaxInstanceContributionToHitGroupIndex() const;
		void							Reset();

		// Num miss and callabale shaders should be configured from the owning module
		UINT							NumMissShaderSlots = 1u;
		UINT							NumCallableShaderSlots = 0u;
	private:

		D_RENDERER_RT_UTILS::TopLevelAccelerationStructure mTLAS;
		D_CONTAINERS::DMap<D_CORE::Uuid, BottomLevelAccelerationStructure> mVBottomLevelAS;
		D_GRAPHICS_BUFFERS::StructuredUploadBuffer<BottomLevelAccelerationStructureInstanceDesc> mBottomLevelASInstanceDescs;
		D_CONTAINERS::DVector<D_CORE::Uuid>			mInstancesUuid;
		UINT                                        mNumBottomLevelASInstances = 0;
		D_GRAPHICS_BUFFERS::ByteAddressBuffer       mAccelerationStructureScratch;
		UINT64                                      mScratchResourceSize = 0;
		UINT64                                      mASmemoryFootprint = 0;

		UINT										mNumShaderSlotPerGeometrySegment;
		UINT										mNumTotalSegments;

		D_CONTAINERS::DUnorderedMap<D_GRAPHICS_UTILS::RayTracingStateObject const*, Utils::ShaderTable*> mShaderTables;

		bool										mTLASSizeDirty;
	};
}