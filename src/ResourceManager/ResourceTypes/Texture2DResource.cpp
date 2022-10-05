#include "ResourceManager/pch.hpp"
#include "Texture2DResource.hpp"

#include <Core/Filesystem/FileUtils.hpp>
#include <Renderer/RenderDeviceManager.hpp>

namespace Darius::ResourceManager
{
	D_CH_RESOURCE_DEF(Texture2DResource);

	void Texture2DResource::CreateRaw(uint32_t color, DXGI_FORMAT format, size_t rowPitchByte, size_t width, size_t height)
	{
		mTexture.Create2D(rowPitchByte, width, height, format, &color);
		auto a = std::shared_ptr<Resource>(new Texture2DResource(GenerateUuid(), Path(), 0, false));
	}

	bool Texture2DResource::UploadToGpu(D_GRAPHICS::GraphicsContext& context)
	{
		auto path = GetPath();
		auto ext = path.extension().string();
		if (ext == ".dds")
		{
			auto fileData = D_FILE::ReadFileSync(path.wstring());
			return mTexture.CreateDDSFromMemory(fileData->data(), fileData->size(), true);
		}
		else if (ext == ".tga")
		{
			auto fileData = D_FILE::ReadFileSync(path.wstring());
			mTexture.CreateTGAFromMemory(fileData->data(), fileData->size(), true);

			return true;
		}

		return false;
	}

}