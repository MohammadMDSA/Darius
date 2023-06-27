#pragma once

#include "Graphics/FrameResource.hpp"
#include "Graphics/GraphicsUtils/Buffers/GpuBuffer.hpp"
#include "Graphics/GraphicsUtils/Buffers/UploadBuffer.hpp"
#include "Graphics/GraphicsUtils/Memory/DescriptorHeap.hpp"
#include "TextureResource.hpp"

#include <ResourceManager/Resource.hpp>
#include <ResourceManager/ResourceRef.hpp>

#include "TerrainResource.generated.hpp"

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif // !D_GRAPHICS

namespace Darius::Graphics
{
	class DClass(Serialize, Resource) TerrainResource : public D_RESOURCE::Resource
	{
		D_CH_RESOURCE_BODY(TerrainResource, "Terrain", ".terrain");

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		void							SetHeightMap(D_RESOURCE::ResourceHandle handle);
		virtual bool					IsDirtyGPU() const override;

		INLINE void						SetHeightFactor(float value)
		{
			mHeightFactor = value;
			MakeDiskDirty();
			MakeGpuDirty();
		}

		INLINE D3D12_GPU_VIRTUAL_ADDRESS GetParamsConstantsAddress() const { return mParametersConstantsGPU
		.GetGpuVirtualAddress(); }
		INLINE D3D12_GPU_DESCRIPTOR_HANDLE	GetTexturesHandle() const { return mTexturesHeap; }

		INLINE operator D_CORE::CountedOwner() {
			return D_CORE::CountedOwner{ GetName(), rttr::type::get<TerrainResource>(), this, 0, [&]() { MakeGpuDirty(); } };
		}


	protected:

		virtual void					WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void					ReadResourceFromFile(D_SERIALIZATION::Json const& j) override;
		virtual bool					UploadToGpu() override;
		virtual INLINE void				Unload() override { EvictFromGpu(); }

	private:

		TerrainResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault = false) :
			Resource(uuid, path, name, id, isDefault),
			mHeightFactor(300.f),
			mHeightMap(GetAsCountedOwner()) {}

		
		DField(Get[const, &, inline])
		D_RESOURCE::ResourceRef<TextureResource>		mHeightMap;

		DField(Get[const, &, inline])
		float											mHeightFactor;

		D_GRAPHICS_BUFFERS::UploadBuffer				mParametersConstantsCPU;
		D_GRAPHICS_BUFFERS::ByteAddressBuffer			mParametersConstantsGPU;

		D_GRAPHICS_MEMORY::DescriptorHandle				mTexturesHeap;

	public:
		Darius_Graphics_TerrainResource_GENERATED
	};
}

File_TerrainResource_GENERATED
