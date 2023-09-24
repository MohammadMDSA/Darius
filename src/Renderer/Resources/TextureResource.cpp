#include "Renderer/pch.hpp"
#include "TextureResource.hpp"

#include <Core/Serialization/TypeSerializer.hpp>
#include <Graphics/CommandContext.hpp>
#include <ResourceManager/ResourceManager.hpp>

#ifdef _D_EDITOR
#include <imgui.h>
#endif

#include "TextureResource.sgenerated.hpp"

using namespace D_GRAPHICS_UTILS;

namespace Darius::Renderer
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

		mCreatedManually = true;

		MakeGpuDirty();

		SignalChange();
	}

	void TextureResource::CreateCubeMap(uint32_t* color, DXGI_FORMAT format, size_t rowPitchByte, size_t width, size_t height)
	{
		mTexture.CreateCube(rowPitchByte, width, height, format, color);

		mCreatedManually = true;

		MakeGpuDirty();

		SignalChange();
	}

#ifdef _D_EDITOR
	bool TextureResource::DrawDetails(float params[])
	{
		D_H_DETAILS_DRAW_BEGIN_TABLE();

		{
			D_H_DETAILS_DRAW_PROPERTY("sRGB");
			bool val = IsSRGB();
			if (ImGui::Checkbox("##sRGB", &val))
			{
				SetSRGB(val);
			}
		}

		auto const& meta = mTexture.GetMeta();

		// UVW Addressing
		{
			static char const* const addressingLabels[] = { "Repeat", "Mirror", "Clamp", "Border", "Mirror Once" };
			static const D3D12_TEXTURE_ADDRESS_MODE addressingValues[] = { D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_MIRROR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE };
			constexpr int addressingValueCount = sizeof(addressingValues) / sizeof(addressingValues[0]);

			// All axes
			{
				D3D12_TEXTURE_ADDRESS_MODE addressing = GetUAddressing();
				bool same = addressing == GetVAddressing() && addressing == GetWAddressing();

				UINT selectedIndex = 0;
				if (same)
					selectedIndex = (UINT)std::distance(addressingValues, std::find(addressingValues, addressingValues + addressingValueCount, addressing));

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
			if (meta.Dimension >= D_GRAPHICS_BUFFERS::Texture::TextureMeta::TEX_DIMENSION_TEXTURE2D)
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
			if (meta.Dimension >= D_GRAPHICS_BUFFERS::Texture::TextureMeta::TEX_DIMENSION_TEXTURE3D)
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

			// Border color
			{
				auto uAddressing = GetUAddressing();
				if (GetUAddressing() == D3D12_TEXTURE_ADDRESS_MODE_BORDER || GetVAddressing() == D3D12_TEXTURE_ADDRESS_MODE_BORDER || GetWAddressing() == D3D12_TEXTURE_ADDRESS_MODE_BORDER)
				{
					auto color = GetBorderColor();
					D_H_DETAILS_DRAW_PROPERTY("Border Color");
					if (ImGui::ColorEdit4("##BorderColor", color.GetPtr()))
					{
						SetBorderColor(color);
					}
				}
			}
		}

		// Filtering Method
		{
			// Creating array of available filters and their labels
			static char const* const filterLabels[] = { "Point", "Bilinear", "Trilinear", "Anisotropic" };

			auto filter = GetFilter();
			int selectedIndex = (int)filter;

			char const* selected = filterLabels[selectedIndex];
			D_H_DETAILS_DRAW_PROPERTY("Filter Mode");
			if (ImGui::BeginCombo("##FilterMode", selected))
			{
				for (UINT i = 0u; i < (UINT)TextureFilterType::Count; i++)
				{
					auto val = (TextureFilterType)i;
					if (ImGui::Selectable(filterLabels[i], val == filter))
					{
						SetFilter(val);
					}
				}

				ImGui::EndCombo();
			}
		}

		// Anisotropic Level
		if (mFilter == TextureFilterType::Anisotropic)
		{
			D_H_DETAILS_DRAW_PROPERTY("Aniso Level");
			int val = (int)mAnisotropicLevel;
			if (ImGui::SliderInt("##AnisotropicLevel", &val, 0, 16))
			{
				SetAnisotropicLevel((UINT)val);
			}
		}

		ImGui::NewLine();

		// Width
		{
			D_H_DETAILS_DRAW_PROPERTY("Width");
			size_t val = mTexture.GetWidth();
			ImGui::Text(std::to_string(val).c_str());
		}

		// Height
		{
			D_H_DETAILS_DRAW_PROPERTY("Height");
			size_t val = mTexture.GetHeight();
			ImGui::Text(std::to_string(val).c_str());
		}

		// Depth
		{
			D_H_DETAILS_DRAW_PROPERTY("Depth");
			size_t val = mTexture.GetDepth();
			ImGui::Text(std::to_string(val).c_str());
		}

		if (meta.Initialized)
		{
			// ArraySize
			{
				D_H_DETAILS_DRAW_PROPERTY("Array Size");
				size_t val = meta.ArraySize;
				ImGui::Text(std::to_string(val).c_str());
			}

			// MipLevels
			{
				D_H_DETAILS_DRAW_PROPERTY("Mip Levels");
				size_t val = meta.MipLevels;
				ImGui::Text(std::to_string(val).c_str());
			}

			// Format
			{
				D_H_DETAILS_DRAW_PROPERTY("Format");
				ImGui::Text(D_GRAPHICS_BUFFERS::Texture::GetFormatString(meta.Format).c_str());
			}

			// Dimension
			{
				D_H_DETAILS_DRAW_PROPERTY("Dimension");
				std::string val;
				switch (meta.Dimension)
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

		return true;
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

			mCreatedManually = false;

		}
		else if (ext == ".tga")
		{
			auto fileData = D_FILE::ReadFileSync(path.wstring());
			mTexture.CreateTGAFromMemory(fileData->data(), fileData->size(), IsSRGB());

			mCreatedManually = false;
		}

		return false;
	}


	void TextureResource::Unload()
	{
		EvictFromGpu();
	}

	void TextureResource::SetUAddressing(D3D12_TEXTURE_ADDRESS_MODE value)
	{
		if (mUAddressing == value)
			return;

		mUAddressing = value;

		mDirtySampler = true;

		MakeGpuDirty();
		MakeDiskDirty();

		SignalChange();
	}

	void TextureResource::SetVAddressing(D3D12_TEXTURE_ADDRESS_MODE value)
	{
		if (mVAddressing == value)
			return;

		mVAddressing = value;

		mDirtySampler = true;

		MakeGpuDirty();
		MakeDiskDirty();

		SignalChange();
	}

	void TextureResource::SetWAddressing(D3D12_TEXTURE_ADDRESS_MODE value)
	{
		if (mWAddressing == value)
			return;

		mWAddressing = value;

		mDirtySampler = true;

		MakeGpuDirty();
		MakeDiskDirty();

		SignalChange();
	}

	void TextureResource::SetAnisotropicLevel(UINT value)
	{
		value = D_MATH::Clamp(value, 0u, 16u);

		if (mAnisotropicLevel == value)
			return;;

		mAnisotropicLevel = value;

		mDirtySampler = true;

		MakeGpuDirty();
		MakeDiskDirty();

		SignalChange();
	}

	void TextureResource::SetFilter(TextureFilterType value)
	{
		if (mFilter == value)
			return;

		mFilter = value;

		mDirtySampler = true;

		MakeGpuDirty();
		MakeDiskDirty();

		SignalChange();
	}

	void TextureResource::SetBorderColor(D_MATH::Color const& color)
	{
		if (color == mBorderColor)
			return;

		mBorderColor = color;

		mDirtySampler = true;

		MakeGpuDirty();
		MakeDiskDirty();

		SignalChange();
	}

	void TextureResource::SetSRGB(bool value)
	{
		if (mSRGB == value)
			return;

		mSRGB = value;

		MakeDiskDirty();
		MakeGpuDirty();

		SignalChange();
	}

	D_GRAPHICS_UTILS::SamplerDesc const& TextureResource::GetSamplerDesc()
	{
		if (!mDirtySampler)
			return mSamplerDesc;

		mSamplerDesc = SamplerDesc();
		switch (GetFilter())
		{
		case TextureFilterType::Point:
			mSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
			break;
		default:
		case TextureFilterType::Bilinear:
			mSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case TextureFilterType::Trilinear:
			mSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		case TextureFilterType::Anisotropic:
			mSamplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
			break;
		}

		mSamplerDesc.AddressU = GetUAddressing();
		mSamplerDesc.AddressV = GetVAddressing();
		mSamplerDesc.AddressW = GetWAddressing();
		mSamplerDesc.MaxAnisotropy = GetAnisotropicLevel();
		mSamplerDesc.SetBorderColor(GetBorderColor());
		mDirtySampler = false;
		return mSamplerDesc;
	}
}
