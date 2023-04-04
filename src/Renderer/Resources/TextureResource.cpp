#include "Renderer/pch.hpp"
#include "TextureResource.hpp"

#include "Renderer/CommandContext.hpp"

#include <ResourceManager/ResourceManager.hpp>

#include <imgui.h>

namespace Darius::Graphics
{
	D_CH_RESOURCE_DEF(TextureResource);

	void TextureResource::WriteResourceToFile(D_SERIALIZATION::Json& json) const
	{
		D_H_SERIALIZE(SRGB);
	}

	void TextureResource::ReadResourceFromFile(D_SERIALIZATION::Json const& json)
	{
		D_H_DESERIALIZE(SRGB);
	}

	void TextureResource::CreateRaw(uint32_t color, DXGI_FORMAT format, size_t rowPitchByte, size_t width, size_t height)
	{
		mTexture.Create2D(rowPitchByte, width, height, format, &color);
	}

	void TextureResource::CreateCubeMap(uint32_t* color, DXGI_FORMAT format, size_t rowPitchByte, size_t width, size_t height)
	{
		mTexture.CreateCube(rowPitchByte, width, height, format, color);

	}

#ifdef _D_EDITOR
	bool TextureResource::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		D_H_DETAILS_DRAW_PROPERTY("sRGB");
		bool val = GetSRGB();
		if (ImGui::Checkbox("##sRGB", &val))
			SetSRGB(val);


		// Width
		{
			D_H_DETAILS_DRAW_PROPERTY("Width");
			size_t val = mTexture.mWidth;
			ImGui::Text(std::to_string(val).c_str());
		}

		// Height
		{
			D_H_DETAILS_DRAW_PROPERTY("Height");
			size_t val = mTexture.mHeight;
			ImGui::Text(std::to_string(val).c_str());
		}

		// Depth
		{
			D_H_DETAILS_DRAW_PROPERTY("Depth");
			size_t val = mTexture.mDepth;
			ImGui::Text(std::to_string(val).c_str());
		}

		if (mTexture.mMetaData.Initialized)
		{
			// ArraySize
			{
				D_H_DETAILS_DRAW_PROPERTY("Array Size");
				size_t val = mTexture.mMetaData.ArraySize;
				ImGui::Text(std::to_string(val).c_str());
			}

			// MipLevels
			{
				D_H_DETAILS_DRAW_PROPERTY("Mip Levels");
				size_t val = mTexture.mMetaData.MipLevels;
				ImGui::Text(std::to_string(val).c_str());
			}

			// Format
			{
				D_H_DETAILS_DRAW_PROPERTY("Format");
				ImGui::Text(D_GRAPHICS_BUFFERS::Texture::GetFormatString(mTexture.mMetaData.Format).c_str());
			}

			// Dimension
			{
				D_H_DETAILS_DRAW_PROPERTY("Dimension");
				std::string val;
				switch (mTexture.mMetaData.Dimension)
				{
				case D_GRAPHICS_BUFFERS::Texture::TextureMeta::TEX_DIMENSION_TEXTURE1D:
					val = "1D";
					break;
				case D_GRAPHICS_BUFFERS::Texture::TextureMeta::TEX_DIMENSION_TEXTURE2D:
					val = "2D";
					break;
				case D_GRAPHICS_BUFFERS::Texture::TextureMeta::TEX_DIMENSION_TEXTURE3D:
					val = "3D";
					break;
				default:
					break;
				}
				ImGui::Text(val.c_str());
			}

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

	bool TextureResource::UploadToGpu()
	{
		auto path = GetPath();
		auto ext = path.extension().string();
		if (ext == ".dds")
		{
			auto fileData = D_FILE::ReadFileSync(path.wstring());
			return mTexture.CreateDDSFromMemory(fileData->data(), fileData->size(), GetSRGB());
		}
		else if (ext == ".tga")
		{
			auto fileData = D_FILE::ReadFileSync(path.wstring());
			mTexture.CreateTGAFromMemory(fileData->data(), fileData->size(), GetSRGB());

			return true;
		}

		return false;
	}


	void TextureResource::Unload()
	{
		EvictFromGpu();
	}

}
