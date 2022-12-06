#pragma once

#include "Resource.hpp"

#include <Renderer/GraphicsUtils/Buffers/Texture.hpp>
#include <Utils/Common.hpp>

#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

using namespace D_CORE;
using namespace D_GRAPHICS_BUFFERS;

namespace Darius::ResourceManager
{
	class DResourceManager;

	class TextureResource : public Resource
	{
		D_CH_RESOURCE_BODY(TextureResource, "Texture", ".tga", ".dds")

	public:
		INLINE D_GRAPHICS_BUFFERS::Texture*			ModifyTextureData() { MakeDiskDirty(), MakeGpuDirty(); return &mTexture; }
		INLINE D_GRAPHICS_BUFFERS::Texture const*	GetTextureData() const { return &mTexture; }

#ifdef _D_EDITOR
		bool										DrawDetails(float params[]);
#endif
		
		void										CreateRaw(uint32_t color, DXGI_FORMAT format, size_t rowPitchByte, size_t width, size_t height);

		D_CH_FIELD_ACC(D_GRAPHICS_BUFFERS::Texture, Texture, protected);
		D_CH_FIELD(bool, CubeMap)

	protected:
		TextureResource(Uuid uuid, std::wstring const& path, std::wstring const& name, DResourceId id, bool isDefault = false) :
			Resource(uuid, path, name, id, isDefault) {}


		// Inherited via Resource
		virtual void WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void ReadResourceFromFile(D_SERIALIZATION::Json const& j) override;
		virtual bool UploadToGpu(D_GRAPHICS::GraphicsContext& context);

		virtual void Unload() override;
	};
}