#pragma once

#include "Utils/AccelerationStructure.hpp"
#include "Utils/ShaderTable.hpp"

#include <Graphics/GraphicsUtils/StateObject.hpp>
#include <Graphics/GraphicsUtils/Memory/DescriptorHeap.hpp>

#ifndef D_RENDERER_RT
#define D_RENDERER_RT Darius::Renderer::RayTracing
#endif // !D_RENDERER_RT_UTILS

namespace Darius::Renderer::RayTracing
{
	class RayTracingScene
	{
	public:
		RayTracingScene(UINT maxNumBLASes, UINT numShaderSlotPerGeometry);
		~RayTracingScene();

		void                            AddBottomLevelAS(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, D_RENDERER_RT_UTILS::BottomLevelAccelerationStructureGeometry const& bottomLevelASGeometry, bool allowUpdate = false, bool performUpdateOnBuild = false);
		UINT                            AddBottomLevelASInstance(D_CORE::Uuid const& bottomLevelASUuid, D_MATH::Matrix4 const& transform = D_MATH::Matrix4::Identity, BYTE InstanceMask = 1);
		void                            InitializeTopLevelAS(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, bool allowUpdate = false, bool performUpdateOnBuild = false, const wchar_t* resourceName = nullptr);

		// Call every frame to build acceleration structures
		void                            Build(RayTracingCommandContext& commandList, ID3D12DescriptorHeap* descriptorHeap, UINT frameIndex, bool bForceBuild = false);
		INLINE D_RENDERER_RT_UTILS::BottomLevelAccelerationStructureInstanceDesc const& GetBottomLevelASInstance(UINT bottomLevelASinstanceIndex) const { return mBottomLevelASInstanceDescs[bottomLevelASinstanceIndex]; }

		// Shader Table Funcs
		D_RENDERER_RT_UTILS::ShaderTable* FindExistingShaderTable(D_GRAPHICS_UTILS::RayTracingStateObject const* stateObject) const;
		D_RENDERER_RT_UTILS::ShaderTable* FindOrCreateShaderTable(D_GRAPHICS_UTILS::RayTracingStateObject const* stateObject);
		D_GRAPHICS_BUFFERS::StructuredUploadBuffer<D_RENDERER_RT_UTILS::BottomLevelAccelerationStructureInstanceDesc> const& GetBottomLevelASInstancesBuffer() const { return mBottomLevelASInstanceDescs; }

		INLINE D_RENDERER_RT_UTILS::BottomLevelAccelerationStructure const* GetBottomLevelAS(D_CORE::Uuid const& uuid) const { auto result = mVBottomLevelAS.find(uuid); return result == mVBottomLevelAS.end() ? nullptr : &result->second; }
		INLINE D_RENDERER_RT_UTILS::TopLevelAccelerationStructure const& GetTopLevelAS() const { return mTLAS; }
		INLINE UINT64                   GetASMemoryFootprint() const { return mASmemoryFootprint; }
		INLINE UINT                     GetNumberOfBottomLevelASInstances() const { return mNumBottomLevelASInstances; }
		INLINE UINT						GetTotalNumberOfGeometrySegments() const { return mCumulativeNumInstanceGeom[mNumBottomLevelASInstances]; }

		UINT                            GetMaxInstanceContributionToHitGroupIndex() const;
		void							Reset();

		// Num miss and callabale shaders should be configured from the owning module
		UINT							NumMissShaderSlots = 1u;
		UINT							NumCallableShaderSlots = 0u;
	private:

		D_RENDERER_RT_UTILS::TopLevelAccelerationStructure mTLAS;
		D_CONTAINERS::DMap<D_CORE::Uuid, D_RENDERER_RT_UTILS::BottomLevelAccelerationStructure> mVBottomLevelAS;
		D_GRAPHICS_BUFFERS::StructuredUploadBuffer<D_RENDERER_RT_UTILS::BottomLevelAccelerationStructureInstanceDesc> mBottomLevelASInstanceDescs;
		D_CONTAINERS::DVector<D_CORE::Uuid>			mInstancesUuid;
		UINT                                        mNumBottomLevelASInstances = 0;
		D_GRAPHICS_BUFFERS::ByteAddressBuffer       mAccelerationStructureScratch;
		UINT64                                      mScratchResourceSize = 0;
		UINT64                                      mASmemoryFootprint = 0;

		UINT										mNumShaderSlotPerGeometrySegment;

		// for ith index, number of total geometries for each instance so far including ith
		D_CONTAINERS::DVector<UINT>					mCumulativeNumInstanceGeom;

		D_CONTAINERS::DUnorderedMap<D_GRAPHICS_UTILS::RayTracingStateObject const*, Utils::ShaderTable*> mShaderTables[D_GRAPHICS_DEVICE::gNumFrameResources];

		bool										mTLASSizeDirty;
	};
}