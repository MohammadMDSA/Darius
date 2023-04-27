#include "Renderer/pch.hpp"
#include "TextureResource.hpp"

#include "Renderer/CommandContext.hpp"

#include <Core/Serialization/TypeSerializer.hpp>
#include <ResourceManager/ResourceManager.hpp>

#ifdef _D_EDITOR
#include <imgui.h>
#endif

#include "TextureResource.sgenerated.hpp"

namespace Darius::Graphics
{
	D_CH_RESOURCE_DEF(TextureResource);

	void TextureResource::WriteResourceToFile(D_SERIALIZATION::Json& json) const
	{
		D_SERIALIZATION::Serialize(*this, json);
	}

	void TextureResource::ReadResourceFromFile(D_SERIALIZATION::Json const& json)
	{
		D_SERIALIZATION::Deserialize(*this, json);
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

		{
			D_H_DETAILS_DRAW_PROPERTY("sRGB");
			bool val = IsSRGB();
			if (ImGui::Checkbox("##sRGB", &val))
			{
				SetSRGB(val);
				valueChanged = true;
			}
		}

		// UVW Addressing
		{
			static char const* const addressingLabels[] = { "Repeat", "Mirror", "Clamp", "Border", "Mirror Once"};
			static const D3D12_TEXTURE_ADDRESS_MODE addressingValues[] = { D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_MIRROR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE };
			constexpr int addressingValueCount = sizeof(addressingValues) / sizeof(addressingValues[0]);

			// All axes
			{
				D3D12_TEXTURE_ADDRESS_MODE addressing = GetUAddressing();
				bool same = addressing == GetVAddressing() && addressing == GetWAddressing();

				auto selectedIndex = -1;
				if(same)
					selectedIndex= std::distance(addressingValues, std::find(addressingValues, addressingValues + addressingValueCount, addressing));

				char const* selected = same ? addressingLabels[selectedIndex] : "Differen Values";
				D_H_DETAILS_DRAW_PROPERTY("Addressing");
				if (ImGui::BeginCombo("##AllAxesAddressing", selected))
				{
					for (UINT i = 0; i < addressingValueCount; i++)
					{
						auto val = addressingValues[i];
						if (ImGui::Selectable(addressingLabels[i], val == addressing))
						{
							SetUAddressing(val);
							SetVAddressing(val);
							SetWAddressing(val);
						}
					}

					ImGui::EndCombo();
				}
			}

			// U addressing
			{

				D3D12_TEXTURE_ADDRESS_MODE addressing = GetUAddressing();
				auto selectedIndex = std::distance(addressingValues, std::find(addressingValues, addressingValues + addressingValueCount, addressing));

				char const* selected = addressingLabels[selectedIndex];
				D_H_DETAILS_DRAW_PROPERTY("U axis");
				if (ImGui::BeginCombo("##UAxisAddressing", selected))
				{
					for (UINT i = 0; i < addressingValueCount; i++)
					{
						auto val = addressingValues[i];
						if (ImGui::Selectable(addressingLabels[i], val == addressing))
						{
							SetUAddressing(val);
						}
					}

					ImGui::EndCombo();
				}
			}

			// V addressing
			if (mTexture.mMetaData.Dimension >= D_GRAPHICS_BUFFERS::Texture::TextureMeta::TEX_DIMENSION_TEXTURE2D)
			{

				D3D12_TEXTURE_ADDRESS_MODE addressing = GetVAddressing();
				auto selectedIndex = std::distance(addressingValues, std::find(addressingValues, addressingValues + addressingValueCount, addressing));

				char const* selected = addressingLabels[selectedIndex];
				D_H_DETAILS_DRAW_PROPERTY("V axis");
				if (ImGui::BeginCombo("##VAxisAddressing", selected))
				{
					for (UINT i = 0; i < addressingValueCount; i++)
					{
						auto val = addressingValues[i];
						if (ImGui::Selectable(addressingLabels[i], val == addressing))
						{
							SetVAddressing(val);
						}
					}

					ImGui::EndCombo();
				}
			}

			// W addressing
			if(mTexture.mMetaData.Dimension >= D_GRAPHICS_BUFFERS::Texture::TextureMeta::TEX_DIMENSION_TEXTURE3D)
			{

				D3D12_TEXTURE_ADDRESS_MODE addressing = GetWAddressing();
				auto selectedIndex = std::distance(addressingValues, std::find(addressingValues, addressingValues + addressingValueCount, addressing));

				char const* selected = addressingLabels[selectedIndex];
				D_H_DETAILS_DRAW_PROPERTY("W axis");
				if (ImGui::BeginCombo("##WAxisAddressing", selected))
				{
					for (UINT i = 0; i < addressingValueCount; i++)
					{
						auto val = addressingValues[i];
						if (ImGui::Selectable(addressingLabels[i], val == addressing))
						{
							SetWAddressing(val);
						}
					}

					ImGui::EndCombo();
				}
			}
		}

		// Filtering Method
		{
			// Creating array of available filters and their labels
			static char const* const filterLabels[] = { "Point", "Bilinear", "Trilinear", "Anisotropic" };
			static const D3D12_FILTER filterValues[] = { D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_FILTER_ANISOTROPIC };
			constexpr int filterValueCount = sizeof(filterValues) / sizeof(filterValues[0]);

			D3D12_FILTER filter = GetFilter();
			auto selectedIndex = std::distance(filterValues, std::find(filterValues, filterValues + filterValueCount, filter));

			char const* selected = filterLabels[selectedIndex];
			D_H_DETAILS_DRAW_PROPERTY("Filter Mode");
			if (ImGui::BeginCombo("##FilterMode", selected))
			{
				for (UINT i = 0; i < filterValueCount; i++)
				{
					auto val = filterValues[i];
					if (ImGui::Selectable(filterLabels[i], val == filter))
					{
						SetFilter(val);
					}
				}

				ImGui::EndCombo();
			}
		}

		// Anisotropic Level
		if (mFilter == D3D12_FILTER_ANISOTROPIC)
		{
			D_H_DETAILS_DRAW_PROPERTY("Aniso Level");
			int val = (short)mAnisotropicLevel;
			if (ImGui::SliderInt("##AnisotropicLevel", &val, 0, 16))
			{
				SetAnisotropicLevel((UINT)val);
			}
		}

		ImGui::NewLine();

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
			return mTexture.CreateDDSFromMemory(fileData->data(), fileData->size(), IsSRGB());
		}
		else if (ext == ".tga")
		{
			auto fileData = D_FILE::ReadFileSync(path.wstring());
			mTexture.CreateTGAFromMemory(fileData->data(), fileData->size(), IsSRGB());

			return true;
		}

		return false;
	}


	void TextureResource::Unload()
	{
		EvictFromGpu();
	}

}
