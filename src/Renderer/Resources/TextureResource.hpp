#pragma once

#include "Renderer/GraphicsUtils/Buffers/Texture.hpp"

#include <ResourceManager/Resource.hpp>
#include <Utils/Common.hpp>

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

namespace Darius::Graphics
{
	class DResourceManager;

	class TextureResource : public D_RESOURCE::Resource
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

		D_CH_FIELD_ACC(D_GRAPHICS_BUFFERS::Texture, Texture, protected);
		D_CH_RESOURCE_RW_FIELD_ACC(bool, SRGB, protected);

	protected:
		TextureResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault = false) :
			Resource(uuid, path, name, id, isDefault),
			mSRGB(false) {}


		// Inherited via Resource
		virtual void WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void ReadResourceFromFile(D_SERIALIZATION::Json const& j) override;
		virtual bool UploadToGpu() override;

		virtual void Unload() override;
	};
}