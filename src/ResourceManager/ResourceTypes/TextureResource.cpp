#include "ResourceManager/pch.hpp"
#include "TextureResource.hpp"

#include "ResourceManager/ResourceManager.hpp"

#include <imgui.h>

namespace Darius::ResourceManager
{
	D_CH_RESOURCE_DEF(TextureResource);

	void TextureResource::WriteResourceToFile(D_SERIALIZATION::Json& json) const
	{
		D_H_SERIALIZE(CubeMap);
	}

	void TextureResource::ReadResourceFromFile(D_SERIALIZATION::Json const& json)
	{
		D_H_DESERIALIZE(CubeMap);
	}

	void TextureResource::CreateRaw(uint32_t color, DXGI_FORMAT format, size_t rowPitchByte, size_t width, size_t height)
	{
		mTexture.Create2D(rowPitchByte, width, height, format, &color);
	}

	
#ifdef _D_EDITOR
	bool TextureResource::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();
		D_H_DETAILS_DRAW_PROPERTY("Cube Map");
		if (ImGui::Checkbox("##CubeMap", &mCubeMap))
		{
			valueChanged = true;
		}

		D_H_DETAILS_DRAW_END_TABLE();

		if (valueChanged)
		{
			MakeDiskDirty();
			MakeGpuDirty();
		}

		return valueChanged;
	}
#endif // _D_EDITOR

	bool TextureResource::UploadToGpu(D_GRAPHICS::GraphicsContext& context)
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


	void TextureResource::Unload()
	{
		EvictFromGpu();
	}

}
