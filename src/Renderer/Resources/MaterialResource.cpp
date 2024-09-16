#include "Renderer/pch.hpp"
#include "MaterialResource.hpp"

#include "Renderer/RendererManager.hpp"

#include <Graphics/GraphicsCore.hpp>
#include <Graphics/GraphicsDeviceManager.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceLoader.hpp>
#include <Core/Serialization/Json.hpp>
#include <Utils/Common.hpp>

#ifdef _D_EDITOR
#include <ResourceManager/ResourceDragDropPayload.hpp>
#include <Libs/FontIcon/IconsFontAwesome6.h>

#include <imgui.h>
#endif

#include <fstream>

#include <MaterialResource.sgenerated.hpp>

using namespace D_CORE;
using namespace D_GRAPHICS_UTILS;
using namespace D_MATH;
using namespace D_SERIALIZATION;
using namespace D_RENDERER;
using namespace D_RENDERER_RAST;
using namespace D_RESOURCE;
using namespace DirectX;

inline uint32_t floatToHalf(float f)
{
	const float kF32toF16 = (1.0 / (1ull << 56)) * (1.0 / (1ull << 56)); // 2^-112
	union { float f; uint32_t u; } x;
	x.f = D_MATH::Clamp(f, 0.0f, 1.0f) * kF32toF16;
	return x.u >> 13;
}

namespace Darius::Renderer
{
	D_CH_RESOURCE_DEF(MaterialResource);

	MaterialResource::MaterialResource(Uuid uuid, std::wstring const& path, std::wstring const& name, DResourceId id, D_RESOURCE::Resource* parent, bool isDefault) :
		GenericMaterialResource(uuid, path, name, id, parent, isDefault),
		mPsoFlags(RenderItem::HasPosition | RenderItem::HasNormal | RenderItem::HasTangent | RenderItem::HasUV0),
		mCutout(0)
	{
		SetTextureCount(TextureType::kNumTextures);
		SetSamplerCount(TextureType::kNumTextures);

		SetTextureSlot({ StringId("BaseColorTexture") }, (uint32_t)kBaseColor);
		SetTextureSlot({ StringId("MetallicTexture") }, (uint32_t)kMetallic);
		SetTextureSlot({ StringId("RoughnessTexture") }, (uint32_t)kRoughness);
		SetTextureSlot({ StringId("NormalTexture") }, (uint32_t)kNormal);
		SetTextureSlot({ StringId("EmissionTexture") }, (uint32_t)kEmissive);
		SetTextureSlot({ StringId("AmbientOcclusionTexture") }, (uint32_t)kAmbientOcclusion);
		SetTextureSlot({ StringId("WorldDisplacementTexture") }, (uint32_t)kWorldDisplacement);

		SetSamplerSlot({ StringId("BaseColorSampler") }, (uint32_t)kBaseColor);
		SetSamplerSlot({ StringId("MetallicSampler") }, (uint32_t)kMetallic);
		SetSamplerSlot({ StringId("RoughnessSampler") }, (uint32_t)kRoughness);
		SetSamplerSlot({ StringId("NormalSampler") }, (uint32_t)kNormal);
		SetSamplerSlot({ StringId("EmissiveSampler") }, (uint32_t)kEmissive);
		SetSamplerSlot({ StringId("AmbientOcclusionSampler") }, (uint32_t)kAmbientOcclusion);
		SetSamplerSlot({ StringId("WorldDisplacementSampler") }, (uint32_t)kWorldDisplacement);
	}

	void MaterialResource::WriteResourceToFile(D_SERIALIZATION::Json& j) const
	{
		float* defalb = (float*)&mMaterial.DifuseAlbedo;
		float* fren = (float*)&mMaterial.FresnelR0;
		float* ems = (float*)&mMaterial.Emissive;
		Json data = {
			{ "DefuseAlbedo", std::vector<float>(defalb, defalb + 4)},
			{ "FresnelR0", std::vector<float>(fren, fren + 3) },
			{ "Roughness", mMaterial.Roughness },
			{ "Metallic", mMaterial.Metallic },
			{ "Emission", std::vector<float>(ems, ems + 3) },
			{ "AlphaCutout", mCutout },
			{ "Opacity", mMaterial.Opacity },
			{ "Specular", mMaterial.Specular },
			{ "DisplacementAmount", mMaterial.DisplacementAmount }
		};

		bool usedBaseColorTex = mMaterial.TextureStatusMask & (1 << kBaseColor);
		if (usedBaseColorTex)
			data["BaseColorTexture"] = ToString(GetTexture(kBaseColor)->GetUuid());

		bool usedMetallicTex = mMaterial.TextureStatusMask & (1 << kMetallic);
		if (usedMetallicTex)
			data["MetallicTexture"] = ToString(GetTexture(kMetallic)->GetUuid());

		bool usedRoughnessTex = mMaterial.TextureStatusMask & (1 << kRoughness);
		if (usedRoughnessTex)
			data["RoughnessTexture"] = ToString(GetTexture(kRoughness)->GetUuid());

		bool usedNormalTex = mMaterial.TextureStatusMask & (1 << kNormal);
		if (usedNormalTex)
			data["NormalTexture"] = ToString(GetTexture(kNormal)->GetUuid());

		bool usedEmissionTex = mMaterial.TextureStatusMask & (1 << kEmissive);
		if (usedEmissionTex)
			data["EmissionTexture"] = ToString(GetTexture(kEmissive)->GetUuid());

		bool usedAmbientOcclusionTex = mMaterial.TextureStatusMask & (1 << kAmbientOcclusion);
		if (usedAmbientOcclusionTex)
			data["AmbientOcclusionTexture"] = ToString(GetTexture(kAmbientOcclusion)->GetUuid());

		bool usedWorldDisplacementTex = mMaterial.TextureStatusMask & (1 << kWorldDisplacement);
		if (usedWorldDisplacementTex)
			data["WorldDisplacementTexture"] = ToString(GetTexture(kWorldDisplacement)->GetUuid());

		data["PsoFlags"] = mPsoFlags;

		std::ofstream os(GetPath());
		os << data;
		os.close();
	}

	void MaterialResource::SetTexture(TextureResource* texture, uint32_t textureIndex)
	{
		if (textureIndex >= GetTextureCount())
		{
			D_LOG_WARN_FMT("Tried to set texture with index {}, but texture count is {}. Ignoring...", (int)textureIndex, (int)GetTextureCount());

			return;
		}

		if (mTextures[textureIndex].Resource == texture)
			return;

		auto textureType = (TextureType)textureIndex;

		if (texture == nullptr)
		{
			mMaterial.TextureStatusMask &= ~(1 << textureIndex);
			if (textureType == kWorldDisplacement)
				mPsoFlags &= ~RenderItem::HasDisplacement;
		}
		else
		{
			mMaterial.TextureStatusMask |= 1 << textureIndex;
			if (textureType == kWorldDisplacement)
				mPsoFlags |= RenderItem::HasDisplacement;
		}

		GenericMaterialResource::SetTexture(texture, textureIndex);

	}

	void MaterialResource::ReadResourceFromFile(D_SERIALIZATION::Json const& j, bool& dirtyDisk)
	{
		dirtyDisk = false;

		Json data;
		std::ifstream is(GetPath());
		is >> data;
		is.close();

		mMaterial.DifuseAlbedo = XMFLOAT4(data["DefuseAlbedo"].get<std::vector<float>>().data());
		mMaterial.FresnelR0 = XMFLOAT3(data["FresnelR0"].get<std::vector<float>>().data());

		if (data.contains("Roughness"))
			mMaterial.Roughness = data["Roughness"];

		if (data.contains("Metallic"))
			mMaterial.Metallic = data["Metallic"];

		if (data.contains("Opacity"))
			mMaterial.Opacity = data["Opacity"];

		if (data.contains("Specular"))
			mMaterial.Specular = data["Specular"];

		mMaterial.Emissive = XMFLOAT3(data["Emission"].get<std::vector<float>>().data());

		mMaterial.TextureStatusMask = 0;


		mPsoFlags = data.contains("PsoFlags") ? data["PsoFlags"].get<uint16_t>() : 0u;
		mPsoFlags = mPsoFlags | RenderItem::HasPosition | RenderItem::HasNormal | RenderItem::HasTangent | RenderItem::HasUV0;


		if (data.contains("BaseColorTexture"))
		{
			auto tex = D_RESOURCE::GetResourceSync<TextureResource>(FromString(data["BaseColorTexture"]), false);
			SetBaseColorTexture(tex.Get());
		}

		if (data.contains("MetallicTexture"))
		{
			auto tex = D_RESOURCE::GetResourceSync<TextureResource>(FromString(data["MetallicTexture"]), false);
			SetMetallicTexture(tex.Get());
		}

		if (data.contains("RoughnessTexture"))
		{
			auto tex = D_RESOURCE::GetResourceSync<TextureResource>(FromString(data["RoughnessTexture"]), false);
			SetRoughnessTexture(tex.Get());
		}

		if (data.contains("NormalTexture"))
		{
			auto tex = D_RESOURCE::GetResourceSync<TextureResource>(FromString(data["NormalTexture"]), false);
			SetNormalTexture(tex.Get());
		}

		if (data.contains("EmissionTexture"))
		{
			auto tex = D_RESOURCE::GetResourceSync<TextureResource>(FromString(data["EmissionTexture"]), false);
			SetEmissiveTexture(tex.Get());
		}

		if (data.contains("AmbientOcclusionTexture"))
		{
			auto tex = D_RESOURCE::GetResourceSync<TextureResource>(FromString(data["AmbientOcclusionTexture"]), false);
			SetAmbientOcclusionTexture(tex.Get());
		}

		if (data.contains("WorldDisplacementTexture"))
		{
			auto tex = D_RESOURCE::GetResourceSync<TextureResource>(FromString(data["WorldDisplacementTexture"]), false);
			SetWorldDisplacementTexture(tex.Get());
		}

		if (data.contains("AlphaCutout"))
		{
			mCutout = data["AlphaCutout"];
			mMaterial.AlphaCutout = floatToHalf(mCutout);
		}

		if (data.contains("DisplacementAmount"))
		{
			mMaterial.DisplacementAmount = data["DisplacementAmount"];
		}
	}

	void MaterialResource::OnTextureChanged(uint32_t index)
	{
		D_ASSERT(index >= 0 && index < GetTextureCount());

		SetSampler(mTextures[index].Resource->GetDefaultSamplerDesc(), index);
	}

	TextureResource* MaterialResource::GetFallbackTexture(uint32_t textureIndex) const
	{
		TextureType type = (TextureType)textureIndex;

		D_RESOURCE::ResourceHandle textureHandle = D_RESOURCE::EmptyResourceHandle;

		switch (type)
		{
		case TextureType::kBaseColor:
			textureHandle = D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DWhiteOpaque);
			break;
		case TextureType::kMetallic:
			textureHandle = D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DBlackOpaque);
			break;
		case TextureType::kRoughness:
			textureHandle = D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DBlackOpaque);
			break;
		case TextureType::kNormal:
			textureHandle = D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DNormalMap);
			break;
		case TextureType::kEmissive:
			textureHandle = D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DBlackOpaque);
			break;
		case TextureType::kAmbientOcclusion:
			textureHandle = D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DWhiteOpaque);
			break;
		case TextureType::kWorldDisplacement:
			textureHandle = D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DBlackOpaque);
			break;
		default:
			textureHandle = D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DWhiteOpaque);
		}

		return D_RESOURCE::GetResourceSync<TextureResource>(textureHandle).Get();
	}

	// Stand alone texture definitions
#define TexSetterDefinition(type) \
	void MaterialResource::Set##type##Texture(TextureResource* texture) \
	{ \
		SetTexture(texture, (uint32_t)k##type);\
		SetSampler(texture->GetDefaultSamplerDesc(), (uint32_t)k##type); \
	}

	TexSetterDefinition(BaseColor);
	TexSetterDefinition(Metallic);
	TexSetterDefinition(Roughness);
	TexSetterDefinition(AmbientOcclusion);
	TexSetterDefinition(Emissive);
	TexSetterDefinition(Normal);
	TexSetterDefinition(WorldDisplacement);

#undef TexSetterDefinition

#ifdef _D_EDITOR
	bool MaterialResource::DrawDetails(float params[])
	{

#define DrawTexture2DHolder(type) \
{ \
	TextureResource* currentTexture = GetTexture(k##type); \
 \
	bool hasTexture = (mMaterial.TextureStatusMask & (1 << k##type)); \
	auto curName = hasTexture ? currentTexture->GetName() : L"<None>"; \
	const char* selectButtonName = hasTexture ? ICON_FA_IMAGE "##" #type : ICON_FA_SQUARE "##" #type; \
	if (ImGui::Button(selectButtonName)) \
	{ \
		\
		ImGui::OpenPopup("Select " #type); \
		D_LOG_DEBUG("OPEN " #type); \
	} \
	D_H_RESOURCE_DRAG_DROP_DESTINATION(TextureResource, Set##type##Texture); \
		 \
	if (ImGui::BeginPopup("Select " #type)) \
	{ \
		bool nonSel = currentTexture == nullptr; \
		if (ImGui::Selectable("<None>", &nonSel)) \
		{ \
			SetTexture(nullptr, (uint32_t)k##type); \
		} \
			 \
		auto meshes = D_RESOURCE::GetResourcePreviews(TextureResource::GetResourceType()); \
		int idx = 0; \
		for (auto prev : meshes) \
		{ \
			bool selected = currentTexture && prev.Handle.Id == currentTexture->GetId() && prev.Handle.Type == currentTexture->GetType(); \
 \
			auto name = WSTR2STR(prev.Name); \
			ImGui::PushID((name + std::to_string(idx)).c_str()); \
			if (ImGui::Selectable(name.c_str(), &selected)) \
			{ \
				SetTexture(static_cast<TextureResource*>(D_RESOURCE::GetRawResourceSync(prev.Handle)), k##type); \
			} \
			ImGui::PopID(); \
				 \
			idx++; \
		} \
		 \
		ImGui::EndPopup(); \
	} \
} \

		// Material constants
	{

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		float valueChanged = false;


		D_H_DETAILS_DRAW_BEGIN_TABLE();

		D_H_DETAILS_DRAW_PROPERTY("Shader Type");

		std::string typeName;
		if (mPsoFlags & RenderItem::AlphaBlend)
			typeName = "Transparent";
		else if (mPsoFlags & RenderItem::AlphaTest)
			typeName = "Cutout";
		else
			typeName = "Opaque";

		if (ImGui::Button(typeName.c_str(), ImVec2(-1, 0)))
		{
			ImGui::OpenPopup("##ShaderTypeSelecionPopup");
		}
		if (ImGui::BeginPopupContextItem("##ShaderTypeSelecionPopup", ImGuiPopupFlags_NoOpenOverExistingPopup))
		{
			if (ImGui::Selectable("Opaque"))
			{
				mPsoFlags &= ~RenderItem::AlphaBlend;
				mPsoFlags &= ~RenderItem::AlphaTest;
				valueChanged = true;
			}
			if (ImGui::Selectable("Cutout"))
			{
				mPsoFlags &= ~RenderItem::AlphaBlend;
				mPsoFlags |= RenderItem::AlphaTest;
				valueChanged = true;
			}
			if (ImGui::Selectable("Transparent"))
			{
				mPsoFlags |= RenderItem::AlphaBlend;
				mPsoFlags &= ~RenderItem::AlphaTest;
				valueChanged = true;
			}
			ImGui::EndPopup();
		}

		// Albedo
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			DrawTexture2DHolder(BaseColor);
			ImGui::SameLine();
			ImGui::Text("Albedo");
			if (!HasAlbedoTexture())
			{
				auto value = GetAlbedoColor();
				ImGui::TableSetColumnIndex(1);
				if (D_MATH::DrawDetails(value, true, Color::Black))
				{
					SetAlbedoColor(value);
				}
			}
		}

		// Metallic
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			DrawTexture2DHolder(Metallic);
			ImGui::SameLine();
			ImGui::Text("Metallic");
			if (!HasMetallicTexture())
			{
				float value = GetMetallic();
				ImGui::TableSetColumnIndex(1);
				if (ImGui::SliderFloat("##Metallic", &value, 0.f, 1.f, "%.3f"))
					SetMetallic(value);

			}
		}

		// Roughness
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			DrawTexture2DHolder(Roughness);
			ImGui::SameLine();
			ImGui::Text("Roughenss");
			if (!HasRoughnessTexture())
			{
				ImGui::TableSetColumnIndex(1);
				float value = GetRoughness();
				if (ImGui::SliderFloat("##Roughness", &value, 0.f, 1.f, "%.3f"))
					SetRoughness(value);
			}
		}

		// Emission
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			DrawTexture2DHolder(Emissive);
			ImGui::SameLine();
			ImGui::Text("Emission");
			if (!HasEmissiveTexture())
			{
				ImGui::TableSetColumnIndex(1);
				auto value = GetEmissiveColor();
				if (D_MATH::DrawDetails(value, false, Color::Black, true, true))
				{
					SetEmissiveColor(value);
				}
			}
		}

		// Opacity
		if (mPsoFlags & RenderItem::AlphaBlend)
		{
			D_H_DETAILS_DRAW_PROPERTY("Opacity");
			float value = GetOpacity();
			if (ImGui::SliderFloat("##Opacity", &value, 0.f, 1.f, "%.3f"))
			{
				SetOpacity(value);
			}
		}

		// Specular
		{
			D_H_DETAILS_DRAW_PROPERTY("Specular");
			float value = GetSpecular();
			if (ImGui::SliderFloat("##Specular", &value, 0.f, 1.f))
			{
				SetSpecular(value);
			}
		}

		// Normal
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		DrawTexture2DHolder(Normal);
		ImGui::SameLine();
		ImGui::Text("Normal");

		// World Displacement
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			DrawTexture2DHolder(WorldDisplacement);
			ImGui::SameLine();
			ImGui::Text("World Displacement");

			if (HasDisplacement())
			{
				ImGui::TableSetColumnIndex(1);
				float value = GetDisplacementAmount();
				if (ImGui::DragFloat("##DisplacementAmount", &value, 0.05f, 0.f))
				{
					SetDisplacementAmount(value);
				}
			}
		}

		// Two sided
		{
			bool val = mPsoFlags & RenderItem::TwoSided;
			D_H_DETAILS_DRAW_PROPERTY("Two Sided");

			if (ImGui::Checkbox("##TwoSided", &val))
			{
				SetTwoSided(val);
			}
		}

		// Alpha test cutout
		if (mPsoFlags & RenderItem::AlphaTest)
		{
			D_H_DETAILS_DRAW_PROPERTY("Alpha Cutout");
			float value = GetAlphaCutout();
			if (ImGui::SliderFloat("##AlphaCutout", &value, 0, 1, "%.2f"))
			{
				SetAlphaCutout(value);
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
	}

	void MaterialResource::SetTwoSided(bool value)
	{
		if (value == IsTwoSided())
			return;

		if (value)
			mPsoFlags |= RenderItem::TwoSided;
		else
			mPsoFlags &= ~RenderItem::TwoSided;

		MakeDiskDirty();
		MakeGpuDirty();

		SignalChange();
	}

	void MaterialResource::SetAlphaCutout(float value)
	{
		value = D_MATH::Max(0.f, value);

		if (value == GetAlphaCutout())
			return;

		mCutout = value;
		mMaterial.AlphaCutout = floatToHalf(value);

		MakeDiskDirty();
		MakeGpuDirty();

		SignalChange();
	}

	void MaterialResource::SetDisplacementAmount(float value)
	{
		value = D_MATH::Max(0.f, value);

		if (value == mMaterial.DisplacementAmount)
			return;

		mMaterial.DisplacementAmount = value;

		MakeDiskDirty();
		MakeGpuDirty();

		SignalChange();
	}

	void MaterialResource::SetOpacity(float value)
	{
		value = D_MATH::Clamp(value, 0.f, 1.f);

		if (value == mMaterial.Opacity)
			return;

		mMaterial.Opacity = value;

		MakeDiskDirty();
		MakeGpuDirty();

		SignalChange();
	}

	void MaterialResource::SetSpecular(float value)
	{
		value = D_MATH::Clamp(value, 0.f, 1.f);

		if (value == mMaterial.Specular)
			return;

		mMaterial.Specular = value;

		MakeDiskDirty();
		MakeGpuDirty();

		SignalChange();
	}

	void MaterialResource::SetEmissiveColor(D_MATH::Color const& value)
	{
		auto valueVector = D_MATH::Vector3(value);
		if (valueVector.Equals(mMaterial.Emissive))
			return;

		mMaterial.Emissive = (XMFLOAT3)valueVector;

		MakeDiskDirty();
		MakeGpuDirty();

		SignalChange();
	}

	void MaterialResource::SetRoughness(float value)
	{
		value = D_MATH::Clamp(value, 0.f, 1.f);

		if (value == GetRoughness())
			return;

		mMaterial.Roughness = value;

		MakeDiskDirty();
		MakeGpuDirty();

		SignalChange();
	}

	void MaterialResource::SetMetallic(float value)
	{
		value = D_MATH::Clamp(value, 0.f, 1.f);

		if (value == GetMetallic())
			return;

		mMaterial.Metallic = value;

		MakeDiskDirty();
		MakeGpuDirty();

		SignalChange();
	}

	void MaterialResource::SetAlbedoColor(D_MATH::Color const& value)
	{
		auto valueVector = D_MATH::Vector4((DirectX::FXMVECTOR)value);
		if (valueVector == mMaterial.DifuseAlbedo)
			return;

		mMaterial.DifuseAlbedo = valueVector;

		MakeDiskDirty();
		MakeGpuDirty();

		SignalChange();
	}

#endif // _D_EDITOR

}