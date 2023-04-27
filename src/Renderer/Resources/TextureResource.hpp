#pragma once

#include "Renderer/GraphicsUtils/Buffers/Texture.hpp"

#include <ResourceManager/Resource.hpp>
#include <Utils/Common.hpp>

#include "TextureResource.generated.hpp"

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

namespace Darius::Graphics
{
	class DResourceManager;

	class DClass(Serialize, Resource) TextureResource : public D_RESOURCE::Resource
	{
		D_CH_RESOURCE_BODY(TextureResource, "Texture", ".tga", ".dds")

	public:

		INLINE D_GRAPHICS_BUFFERS::Texture*			ModifyTextureData() { MakeDiskDirty(), MakeGpuDirty(); return &mTexture; }
		INLINE D_GRAPHICS_BUFFERS::Texture const*	GetTextureData() const { return &mTexture; }

#ifdef _D_EDITOR
		bool										DrawDetails(float params[]);
#endif
		
		void										CreateRaw(uint32_t color, DXGI_FORMAT format, size_t rowPitchByte, size_t width, size_t height);
		void										CreateCubeMap(uint32_t* color, DXGI_FORMAT format, size_t rowPitchByte, size_t width, size_t height);

		INLINE D3D12_FILTER							GetFilter() const { return (D3D12_FILTER)mFilter; }
		INLINE void									SetFilter(D3D12_FILTER value) { mFilter = value; MakeGpuDirty(); MakeDiskDirty(); }

		INLINE D3D12_TEXTURE_ADDRESS_MODE			GetUAddressing() const { return (D3D12_TEXTURE_ADDRESS_MODE)mUAddressing; }
		INLINE D3D12_TEXTURE_ADDRESS_MODE			GetVAddressing() const { return (D3D12_TEXTURE_ADDRESS_MODE)mVAddressing; }
		INLINE D3D12_TEXTURE_ADDRESS_MODE			GetWAddressing() const { return (D3D12_TEXTURE_ADDRESS_MODE)mWAddressing; }
		INLINE void									SetUAddressing(D3D12_TEXTURE_ADDRESS_MODE value) { mUAddressing = value; MakeGpuDirty(); MakeDiskDirty(); }
		INLINE void									SetVAddressing(D3D12_TEXTURE_ADDRESS_MODE value) { mVAddressing = value; MakeGpuDirty(); MakeDiskDirty(); }
		INLINE void									SetWAddressing(D3D12_TEXTURE_ADDRESS_MODE value) { mWAddressing = value; MakeGpuDirty(); MakeDiskDirty(); }

		INLINE void									SetAnisotropicLevel(UINT value) { mAnisotropicLevel = value; MakeGpuDirty(); MakeDiskDirty(); }

		D_CH_RESOURCE_RW_FIELD_ACC(bool, SRGB, protected, Get[inline], Serialize);


	protected:
		TextureResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault = false) :
			Resource(uuid, path, name, id, isDefault),
			mSRGB(false),
			mFilter(D3D12_FILTER_MIN_MAG_MIP_LINEAR),
			mAnisotropicLevel(16),
			mUAddressing(D3D12_TEXTURE_ADDRESS_MODE_WRAP),
			mVAddressing(D3D12_TEXTURE_ADDRESS_MODE_WRAP),
			mWAddressing(D3D12_TEXTURE_ADDRESS_MODE_WRAP) {}


		// Inherited via Resource
		virtual void								WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void								ReadResourceFromFile(D_SERIALIZATION::Json const& j) override;
		virtual bool								UploadToGpu() override;

		virtual void								Unload() override;

		DField(Serialize)
		UINT										mFilter;

		DField(Get[inline], Serialize)
		UINT										mAnisotropicLevel;
		
		DField(Serialize)
		UINT										mUAddressing;

		DField(Serialize)
		UINT										mVAddressing;

		DField(Serialize)
		UINT										mWAddressing;

		D_GRAPHICS_BUFFERS::Texture					mTexture;

	public:
		Darius_Graphics_TextureResource_GENERATED

	};
}

File_TextureResource_GENERATED
