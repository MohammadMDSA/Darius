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
	public:
		INLINE D_GRAPHICS_BUFFERS::Texture*			ModifyTextureData() { MakeDiskDirty(), MakeGpuDirty(); return &mTexture; }
		INLINE D_GRAPHICS_BUFFERS::Texture const*	GetTextureData() const { return &mTexture; }

		D_CH_FIELD_ACC(D_GRAPHICS_BUFFERS::Texture, Texture, protected);

	protected:
		TextureResource(Uuid uuid, std::wstring const& path, std::wstring const& name, DResourceId id, bool isDefault = false) :
			Resource(uuid, path, name, id, isDefault) {}


		// Inherited via Resource
		virtual void WriteResourceToFile() const override;
		virtual void ReadResourceFromFile() override;
		virtual void Unload() override;
	};
}