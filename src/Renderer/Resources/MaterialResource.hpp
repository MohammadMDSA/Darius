#pragma once

#include "TextureResource.hpp"
#include "Renderer/FrameResource.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/GraphicsUtils/Buffers/UploadBuffer.hpp"
#include "Renderer/GraphicsUtils/Buffers/GpuBuffer.hpp"
#include "Renderer/Resources/TextureResource.hpp"

#include <ResourceManager/Resource.hpp>
#include <Core/Ref.hpp>

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

using namespace D_RENDERER_FRAME_RESOUCE;
using namespace D_GRAPHICS_BUFFERS;
using namespace D_CORE;

namespace Darius::Graphics
{
	class DResourceManager;

	class MaterialResource : public D_RESOURCE::Resource
	{
		D_CH_RESOURCE_BODY(MaterialResource, "Material", ".mat")
		
	public:
		INLINE MaterialConstants*			ModifyMaterialData() { MakeDiskDirty(); MakeGpuDirty(); return &mMaterial; }
		INLINE const MaterialConstants*		GetMaterialData() const { return &mMaterial; }
		INLINE D3D12_GPU_DESCRIPTOR_HANDLE	GetTexturesHandle() const { return mTexturesHeap; }
		INLINE uint16_t						GetPsoFlags() const { return mPsoFlags; }
		void								SetTexture(D_RESOURCE::ResourceHandle textureHandle, D_RENDERER::TextureType type);

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif // _D_EDITOR

		INLINE operator const MaterialConstants* () const { return &mMaterial; }
		INLINE operator MaterialConstants*() { ModifyMaterialData(); }

		INLINE operator D3D12_GPU_VIRTUAL_ADDRESS() const { return mMaterialConstantsGPU.GetGpuVirtualAddress(); }

		INLINE operator CountedOwner const() {
			return CountedOwner{ GetName(), "Material Resource", this, 0 };
		}
			
		D_CH_FIELD(MaterialConstants, Material);
		D_CH_R_FIELD(Ref<D_GRAPHICS::TextureResource>, BaseColorTexture);
		D_CH_R_FIELD(Ref<D_GRAPHICS::TextureResource>, NormalTexture);
		D_CH_R_FIELD(Ref<D_GRAPHICS::TextureResource>, MetallicTexture);
		D_CH_R_FIELD(Ref<D_GRAPHICS::TextureResource>, RoughnessTexture);
		D_CH_R_FIELD(Ref<D_GRAPHICS::TextureResource>, EmissiveTexture);
		D_CH_R_FIELD(D_RESOURCE::ResourceHandle, BaseColorTextureHandle);
		D_CH_R_FIELD(D_RESOURCE::ResourceHandle, NormalTextureHandle);
		D_CH_R_FIELD(D_RESOURCE::ResourceHandle, MetallicTextureHandle);
		D_CH_R_FIELD(D_RESOURCE::ResourceHandle, RoughnessTextureHandle);
		D_CH_R_FIELD(D_RESOURCE::ResourceHandle, EmissiveTextureHandle);

	protected:

		virtual void						WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void						ReadResourceFromFile(D_SERIALIZATION::Json const& j) override;
		virtual bool						UploadToGpu(void* context) override;
		INLINE virtual void					Unload() override { EvictFromGpu(); }

	private:
		friend class DResourceManager;

		MaterialResource(Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault = false);

		D_GRAPHICS_BUFFERS::UploadBuffer	mMaterialConstantsCPU[D_RENDERER_FRAME_RESOUCE::gNumFrameResources];
		ByteAddressBuffer					mMaterialConstantsGPU;

		DescriptorHandle					mTexturesHeap;

		uint16_t							mPsoFlags;

	};
}