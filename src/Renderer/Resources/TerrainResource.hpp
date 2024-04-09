#pragma once

#include "TextureResource.hpp"

#include "Renderer/Resources/StaticMeshResource.hpp"

#include <Graphics/GraphicsUtils/Memory/DescriptorHeap.hpp>
#include <Graphics/GraphicsUtils/Buffers/GpuBuffer.hpp>
#include <Graphics/GraphicsUtils/Buffers/UploadBuffer.hpp>
#include <ResourceManager/Resource.hpp>
#include <ResourceManager/ResourceRef.hpp>
#include <Utils/StaticConstructor.hpp>

#include "TerrainResource.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DClass(Serialize, Resource) TerrainResource : public D_RESOURCE::Resource
	{
		GENERATED_BODY();
		D_CH_RESOURCE_BODY(TerrainResource, "Terrain", ".terrain");

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		void							SetHeightMap(TextureResource* hightMap);
		INLINE TextureResource*			GetHeightMap() const { return mHeightMap.Get(); }

		void							SetHeightFactor(float value);
		INLINE float					GetHeightFactor() const { return mHeightFactor; }
		

		INLINE D3D12_GPU_VIRTUAL_ADDRESS GetParamsConstantsAddress() const { return mParametersConstantsGPU
		.GetGpuVirtualAddress(); }
		INLINE D3D12_GPU_DESCRIPTOR_HANDLE	GetTexturesHandle() const { return mTexturesHeap; }

		INLINE D_RENDERER_GEOMETRY::Mesh const& GetMeshData() const { return mMesh; }

		INLINE virtual bool				AreDependenciesDirty() const override { return mHeightMap.IsValidAndGpuDirty(); }

		void							GetDimensions(float& width, float& height);


	protected:

		virtual void					WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void					ReadResourceFromFile(D_SERIALIZATION::Json const& j, bool& dirtyDisk) override;
		virtual bool					UploadToGpu() override;
		virtual INLINE void				Unload() override { EvictFromGpu(); }

		void							UpdateBoundsMath();
	private:

		TerrainResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault = false);

		bool							InitRasterization();
		bool							InitRayTracing();

		
		DField()
		D_RESOURCE::ResourceRef<TextureResource>		mHeightMap;

		DField()
		float											mHeightFactor;

		// Rasterization Stuff
		D_GRAPHICS_BUFFERS::UploadBuffer				mParametersConstantsCPU;
		D_GRAPHICS_BUFFERS::ByteAddressBuffer			mParametersConstantsGPU;
		D_GRAPHICS_MEMORY::DescriptorHandle				mTexturesHeap;

		// RayTracing Stuff
		D_RENDERER_GEOMETRY::Mesh						mMesh;

		D_GRAPHICS_BUFFERS::Texture						mGeneratedNormalMap;

	};
}

File_TerrainResource_GENERATED
