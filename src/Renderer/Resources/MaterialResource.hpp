#pragma once

#include "TextureResource.hpp"
#include "Renderer/FrameResource.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/GraphicsUtils/Buffers/UploadBuffer.hpp"
#include "Renderer/GraphicsUtils/Buffers/GpuBuffer.hpp"
#include "Renderer/Resources/TextureResource.hpp"

#include <ResourceManager/Resource.hpp>
#include <Core/Ref.hpp>

#include "MaterialResource.generated.hpp"

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

namespace Darius::Graphics
{
	class DResourceManager;

	class DClass(Serialize, Resource) MaterialResource : public D_RESOURCE::Resource
	{
		D_CH_RESOURCE_BODY(MaterialResource, "Material", ".mat")
		
	public:

		INLINE D_RENDERER_FRAME_RESOURCE::MaterialConstants* ModifyMaterialData() { MakeDiskDirty(); MakeGpuDirty(); return &mMaterial; }
		INLINE const D_RENDERER_FRAME_RESOURCE::MaterialConstants* GetMaterialData() const { return &mMaterial; }
		INLINE D3D12_GPU_DESCRIPTOR_HANDLE	GetTexturesHandle() const { return mTexturesHeap; }
		INLINE D3D12_GPU_DESCRIPTOR_HANDLE	GetSamplersHandle() const { return mSamplerTable; }
		INLINE uint16_t						GetPsoFlags() const { return mPsoFlags; }
		void								SetTexture(D_RESOURCE::ResourceHandle textureHandle, D_RENDERER::TextureType type);

		virtual bool						IsDirtyGPU() const override;

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif // _D_EDITOR

		INLINE operator const D_RENDERER_FRAME_RESOURCE::MaterialConstants* () const { return &mMaterial; }
		INLINE operator D_RENDERER_FRAME_RESOURCE::MaterialConstants*() { ModifyMaterialData(); }

		INLINE operator D3D12_GPU_VIRTUAL_ADDRESS() const { return mMaterialConstantsGPU.GetGpuVirtualAddress(); }

		INLINE operator D_CORE::CountedOwner const() {
			return D_CORE::CountedOwner{ GetName(), rttr::type::get<MaterialResource>(), this, 0};
		}

#define TextureSetter(type) \
inline void Set##type##Texture(D_RESOURCE::ResourceHandle textureHandle) { SetTexture(textureHandle, D_RENDERER::k##type); }
			
		TextureSetter(BaseColor);
		TextureSetter(Metallic);
		TextureSetter(Roughness);
		TextureSetter(AmbientOcclusion);
		TextureSetter(Emissive);
		TextureSetter(Normal);
#undef TextureSetter

	protected:

		virtual void						WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void						ReadResourceFromFile(D_SERIALIZATION::Json const& j) override;
		virtual bool						UploadToGpu() override;
		INLINE virtual void					Unload() override { EvictFromGpu(); }

	private:
		friend class DResourceManager;

		MaterialResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault = false);


		
		DField(Get[const, &, inline])
		D_RESOURCE::ResourceRef<D_GRAPHICS::TextureResource>	mBaseColorTexture;
		
		DField(Get[const, &, inline])
		D_RESOURCE::ResourceRef<D_GRAPHICS::TextureResource>	mNormalTexture;
		
		DField(Get[const, &, inline])
		D_RESOURCE::ResourceRef<D_GRAPHICS::TextureResource>	mMetallicTexture;
		
		DField(Get[const, &, inline])
		D_RESOURCE::ResourceRef<D_GRAPHICS::TextureResource>	mRoughnessTexture;
		
		DField(Get[const, &, inline])
		D_RESOURCE::ResourceRef<D_GRAPHICS::TextureResource>	mEmissiveTexture;
		
		DField(Get[const, &, inline])
		D_RESOURCE::ResourceRef<D_GRAPHICS::TextureResource>	mAmbientOcclusionTexture;
		
		DField(Get[const, &, inline])
		D_RESOURCE::ResourceHandle					mBaseColorTextureHandle;
		
		DField(Get[const, &, inline])
		D_RESOURCE::ResourceHandle					mNormalTextureHandle;
		
		DField(Get[const, &, inline])
		D_RESOURCE::ResourceHandle					mMetallicTextureHandle;
		
		DField(Get[const, &, inline])
		D_RESOURCE::ResourceHandle					mRoughnessTextureHandle;
		
		DField(Get[const, &, inline])
		D_RESOURCE::ResourceHandle					mEmissiveTextureHandle;

		DField(Get[const, &, inline])
		D_RESOURCE::ResourceHandle					mAmbientOcclusionTextureHandle;


		D_RENDERER_FRAME_RESOURCE::MaterialConstants mMaterial;
		D_GRAPHICS_BUFFERS::UploadBuffer			mMaterialConstantsCPU[D_RENDERER_FRAME_RESOURCE::gNumFrameResources];
		D_GRAPHICS_BUFFERS::ByteAddressBuffer		mMaterialConstantsGPU;

		D_GRAPHICS_MEMORY::DescriptorHandle			mTexturesHeap;
		D_GRAPHICS_MEMORY::DescriptorHandle			mSamplerTable;

		uint16_t									mPsoFlags;

		float										mCutout;

	public:
		Darius_Graphics_MaterialResource_GENERATED

	};
}

File_MaterialResource_GENERATED