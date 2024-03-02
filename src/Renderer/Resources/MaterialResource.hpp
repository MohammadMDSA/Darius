#pragma once

#include "Renderer/Rasterization/Renderer.hpp"
#include "Renderer/RendererCommon.hpp"
#include "TextureResource.hpp"

#include <Graphics/GraphicsUtils/Buffers/GpuBuffer.hpp>
#include <Graphics/GraphicsUtils/Buffers/UploadBuffer.hpp>
#include <ResourceManager/Resource.hpp>
#include <Core/RefCounting/Ref.hpp>

#include "MaterialResource.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DResourceManager;

	class DClass(Serialize, Resource) MaterialResource : public D_RESOURCE::Resource
	{
		GENERATED_BODY();
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
		void								SetTexture(TextureResource* texture, D_RENDERER_RAST::TextureType type);

		// Two sided
		INLINE bool							IsTwoSided() const { return mPsoFlags & RenderItem::TwoSided; }
		void								SetTwoSided(bool value);

		INLINE float						GetAlphaCutout() const { return mCutout; }
		void								SetAlphaCutout(float value);

		// Displacement
		INLINE bool							HasDisplacement() const { return (mMaterial.TextureStatusMask & (1 << D_RENDERER_RAST::kWorldDisplacement)) != 0; }
		INLINE float						GetDisplacementAmount() const { return mMaterial.DisplacementAmount; }
		void								SetDisplacementAmount(float value);

		// Emissive
		INLINE D_MATH::Color				GetEmissiveColor() const { return D_MATH::Color(D_MATH::Vector3(mMaterial.Emissive)); }
		void								SetEmissiveColor(D_MATH::Color const& value);
		INLINE bool							HasEmissiveTexture() const { return mMaterial.TextureStatusMask & (1 << D_RENDERER_RAST::kEmissive); }

		// Roughness
		INLINE float						GetRoughness() const { return mMaterial.Roughness; }
		void								SetRoughness(float value);
		INLINE bool							HasRoughnessTexture() const { return mMaterial.TextureStatusMask & (1 << D_RENDERER_RAST::kRoughness); }

		// Metallic
		INLINE float						GetMetallic() const { return mMaterial.Metallic; }
		void								SetMetallic(float value);
		INLINE bool							HasMetallicTexture() const { return mMaterial.TextureStatusMask & (1 << D_RENDERER_RAST::kMetallic); }

		// Albedo
		INLINE D_MATH::Color				GetAlbedoColor() const { return D_MATH::Color(D_MATH::Vector4(mMaterial.DifuseAlbedo)); }
		void								SetAlbedoColor(D_MATH::Color const& value);
		INLINE bool							HasAlbedoTexture() const { return mMaterial.TextureStatusMask & (1 << D_RENDERER_RAST::kBaseColor); }


#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif // _D_EDITOR

		INLINE operator const D_RENDERER::MaterialConstants* () const { return &mMaterial; }
		INLINE operator D_RENDERER::MaterialConstants*() { ModifyMaterialData(); }

		INLINE operator D3D12_GPU_VIRTUAL_ADDRESS() const { return mMaterialConstantsGPU.GetGpuVirtualAddress(); }
		virtual bool						AreDependenciesDirty() const override;


		// Stand alone texture setters declarations
#define TextureSetter(type) \
		void Set##type##Texture(TextureResource* texture);
			
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
		virtual void						ReadResourceFromFile(D_SERIALIZATION::Json const& j, bool& dirtyDisk) override;
		virtual bool						UploadToGpu() override;
		INLINE virtual void					Unload() override { EvictFromGpu(); }

	private:
		friend class DResourceManager;

		MaterialResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault = false);
		
		D_RESOURCE::ResourceRef<D_RENDERER::TextureResource>	mBaseColorTexture;
		
		D_RESOURCE::ResourceRef<D_RENDERER::TextureResource>	mNormalTexture;
		
		D_RESOURCE::ResourceRef<D_RENDERER::TextureResource>	mMetallicTexture;
		
		D_RESOURCE::ResourceRef<D_RENDERER::TextureResource>	mRoughnessTexture;
		
		D_RESOURCE::ResourceRef<D_RENDERER::TextureResource>	mEmissiveTexture;
		
		D_RESOURCE::ResourceRef<D_RENDERER::TextureResource>	mAmbientOcclusionTexture;
		
		D_RESOURCE::ResourceRef<D_RENDERER::TextureResource>	mWorldDisplacementTexture;
		

		D_RENDERER::MaterialConstants				mMaterial;
		D_GRAPHICS_BUFFERS::UploadBuffer			mMaterialConstantsCPU;
		D_GRAPHICS_BUFFERS::ByteAddressBuffer		mMaterialConstantsGPU;

		D_GRAPHICS_MEMORY::DescriptorHandle			mTexturesHeap;
		D_GRAPHICS_MEMORY::DescriptorHandle			mSamplerTable;

		uint16_t									mPsoFlags;

		float										mCutout;

	};
}

File_MaterialResource_GENERATED