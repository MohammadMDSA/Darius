#pragma once

#include "TextureResource.hpp"


#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

namespace Darius::ResourceManager
{
	class DResourceManager;

	class Texture2DResource : public TextureResource
	{
		D_CH_RESOURCE_BODY(Texture2DResource, ResourceType::Texture2D)

	protected:

		virtual bool UploadToGpu(D_GRAPHICS::GraphicsContext& context);

	private:

		friend class DResourceManager;

		Texture2DResource(Uuid uuid, std::wstring const& path, DResourceId id, bool isDefault = false) :
			TextureResource(uuid, path, id, isDefault) {}

		void			CreateRaw(uint32_t color, DXGI_FORMAT format, size_t rowPitchByte, size_t width, size_t height);
	};
}