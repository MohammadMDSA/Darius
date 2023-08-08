#pragma once

#include "Renderer/Rasterization/Renderer.hpp"
#include "Renderer/RendererCommon.hpp"
#include "TextureResource.hpp"

#include <Graphics/GraphicsUtils/Buffers/GpuBuffer.hpp>
#include <Graphics/GraphicsUtils/Buffers/UploadBuffer.hpp>
#include <ResourceManager/Resource.hpp>
#include <Core/Ref.hpp>

#include "MaterialResource.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DResourceManager;

	class DClass(Serialize, Resource) MaterialResource : public D_RESOURCE::Resource
	{
		D_CH_RESOURCE_BODY(MaterialResource, "Material", ".mat")
		
	public:

		INLINE D_RENDERER::MaterialConstants* ModifyMaterialData() { MakeDiskDirty(); MakeGpuDirty(); return &mMaterial; }
		INLINE const D_RENDERER::MaterialConstants* GetMaterialData() const { return &mMaterial; }
		INLINE D3D12_GPU_DESCRIPTOR_HANDLE	GetTexturesHandle() const { return mTexturesHeap; }
		INLINE D3D12_GPU_DESCRIPTOR_HANDLE	GetSamplersHandle() const { return mSamplerTable; }
		INLINE D3D12_GPU_VIRTUAL_ADDRESS	GetConstantsGpuAddress() const { return mMaterialConstantsGPU.GetGpuVirtualAddress(); }
		INLINE uint16_t						GetPsoFlags() const
		{
			return mPsoFlags
#ifdef _D_EDITOR
				| D_RENDERER_RAST::GetForcedPsoFlags()
#endif
				;
		}
		void								SetTexture(D_RESOURCE::ResourceHandle textureHandle, D_RENDERER_RAST::TextureType type);

		virtual bool						IsDirtyGPU() const override;

		INLINE bool							HasDisplacement() const { return (mMaterial.TextureStatusMask & (1 << D_RENDERER_RAST::kWorldDisplacement)) != 0; }

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif // _D_EDITOR

		INLINE operator const D_RENDERER::MaterialConstants* () const { return &mMaterial; }
		INLINE operator D_RENDERER::MaterialConstants*() { ModifyMaterialData(); }

		INLINE operator D3D12_GPU_VIRTUAL_ADDRESS() const { return mMaterialConstantsGPU.GetGpuVirtualAddress(); }

		INLINE operator D_CORE::CountedOwner const() {
			return D_CORE::CountedOwner{ GetName(), rttr::type::get<MaterialResource>(), this, 0};
		}

#define TextureSetter(type) \
inline void Set##type##Texture(D_RESOURCE::ResourceHandle textureHandle) { SetTexture(textureHandle, D_RENDERER_RAST::k##type); }
			
		TextureSetter(BaseColor);
		TextureSetter(Metallic);
		TextureSetter(Roughness);
		TextureSetter(AmbientOcclusion);
		TextureSetter(Emissive);
		TextureSetter(Normal);
		TextureSetter(WorldDisplacement);
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
		D_RESOURCE::ResourceRef<D_RENDERER::TextureResource>	mBaseColorTexture;
		
		DField(Get[const, &, inline])
		D_RESOURCE::ResourceRef<D_RENDERER::TextureResource>	mNormalTexture;
		
		DField(Get[const, &, inline])
		D_RESOURCE::ResourceRef<D_RENDERER::TextureResource>	mMetallicTexture;
		
		DField(Get[const, &, inline])
		D_RESOURCE::ResourceRef<D_RENDERER::TextureResource>	mRoughnessTexture;
		
		DField(Get[const, &, inline])
		D_RESOURCE::ResourceRef<D_RENDERER::TextureResource>	mEmissiveTexture;
		
		DField(Get[const, &, inline])
		D_RESOURCE::ResourceRef<D_RENDERER::TextureResource>	mAmbientOcclusionTexture;
		
		DField(Get[const, &, inline])
		D_RESOURCE::ResourceRef<D_RENDERER::TextureResource>	mWorldDisplacementTexture;
		
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

		DField(Get[const, &, inline])
		D_RESOURCE::ResourceHandle					mWorldDisplacementTextureHandle;


		D_RENDERER::MaterialConstants				mMaterial;
		D_GRAPHICS_BUFFERS::UploadBuffer			mMaterialConstantsCPU;
		D_GRAPHICS_BUFFERS::ByteAddressBuffer		mMaterialConstantsGPU;

		D_GRAPHICS_MEMORY::DescriptorHandle			mTexturesHeap;
		D_GRAPHICS_MEMORY::DescriptorHandle			mSamplerTable;

		uint16_t									mPsoFlags;

		float										mCutout;

	public:
		Darius_Renderer_MaterialResource_GENERATED

	};
}

File_MaterialResource_GENERATED