#pragma once


#include <Graphics/GraphicsUtils/Buffers/Texture.hpp>
#include <Graphics/GraphicsUtils/SamplerManager.hpp>
#include <Math/Color.hpp>
#include <ResourceManager/Resource.hpp>
#include <Utils/Common.hpp>

#include "TextureResource.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DResourceManager;

	enum class DEnum(Serialize) TextureFilterType : UINT
	{
		Point,
		Bilinear,
		Trilinear,
		Anisotropic,

		Count
	};

	class DClass(Serialize, Resource) TextureResource : public D_RESOURCE::Resource
	{
		GENERATED_BODY();
		D_CH_RESOURCE_BODY(TextureResource, "Texture", ".tga", ".dds", ".png")

	public:

		INLINE D_GRAPHICS_BUFFERS::Texture*			ModifyTextureData() { MakeDiskDirty(), MakeGpuDirty(); return &mTexture; }
		INLINE D_GRAPHICS_BUFFERS::Texture const*	GetTextureData() const { return &mTexture; }

#ifdef _D_EDITOR
		bool										DrawDetails(float params[]);
#endif
		
		void										CreateRaw(uint32_t color, DXGI_FORMAT format, size_t rowPitchByte, size_t width, size_t height);
		void										CreateCubeMap(uint32_t* color, DXGI_FORMAT format, size_t rowPitchByte, size_t width, size_t height);

		// SRGB
		void										SetSRGB(bool value);

		// Border Color
		void										SetBorderColor(D_MATH::Color const& color);

		// Filter
		INLINE TextureFilterType					GetFilter() const { return mFilter; }
		void										SetFilter(TextureFilterType value);

		// Texture Addressing
		INLINE D3D12_TEXTURE_ADDRESS_MODE			GetUAddressing() const { return (D3D12_TEXTURE_ADDRESS_MODE)mUAddressing; }
		INLINE D3D12_TEXTURE_ADDRESS_MODE			GetVAddressing() const { return (D3D12_TEXTURE_ADDRESS_MODE)mVAddressing; }
		INLINE D3D12_TEXTURE_ADDRESS_MODE			GetWAddressing() const { return (D3D12_TEXTURE_ADDRESS_MODE)mWAddressing; }
		void										SetUAddressing(D3D12_TEXTURE_ADDRESS_MODE value);
		void										SetVAddressing(D3D12_TEXTURE_ADDRESS_MODE value);
		void										SetWAddressing(D3D12_TEXTURE_ADDRESS_MODE value);

		// Anisotropic Level
		void										SetAnisotropicLevel(UINT value);

		D_GRAPHICS_UTILS::SamplerDesc				GetDefaultSamplerDesc();

		INLINE virtual bool							AreDependenciesDirty() const override { return false; }

	protected:
		TextureResource(D_CORE::Uuid const& uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault = false) :
			Resource(uuid, path, name, id, parent, isDefault),
			mSRGB(false),
			mFilter(TextureFilterType::Trilinear),
			mAnisotropicLevel(16),
			mUAddressing(D3D12_TEXTURE_ADDRESS_MODE_WRAP),
			mVAddressing(D3D12_TEXTURE_ADDRESS_MODE_WRAP),
			mWAddressing(D3D12_TEXTURE_ADDRESS_MODE_WRAP),
			mBorderColor(D_MATH::Color::Black),
			mSamplerDesc(),
			mDirtySampler(true),
			mCreatedManually(true)
		{}


		// Inherited via Resource
		virtual bool								WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void								ReadResourceFromFile(D_SERIALIZATION::Json const& j, bool& dirtyDisk) override;
		virtual bool								UploadToGpu() override;

		virtual void								Unload() override;
		
		DField(Serialize)
		TextureFilterType							mFilter;

		DField(Get[inline], Serialize)
		UINT										mAnisotropicLevel;
		
		DField(Serialize)
		UINT										mUAddressing;

		DField(Serialize)
		UINT										mVAddressing;

		DField(Serialize)
		UINT										mWAddressing;

		DField(Get[inline])
		bool										mDirtySampler;

		DField(Get[inline])
		bool										mCreatedManually;

		DField(Get[inline, const, &], Serialize)
		D_MATH::Color								mBorderColor;

		DField(Get[inline], Serialize)
		bool										mSRGB;

		D_GRAPHICS_BUFFERS::Texture					mTexture;

		D_GRAPHICS_UTILS::SamplerDesc				mSamplerDesc;

	};
}

File_TextureResource_GENERATED
