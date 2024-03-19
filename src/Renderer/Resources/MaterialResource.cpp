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

	MaterialResource::MaterialResource(Uuid uuid, std::wstring const& path, std::wstring const& name, DResourceId id, bool isDefault) :
		Resource(uuid, path, name, id, isDefault),
		mPsoFlags(RenderItem::HasPosition | RenderItem::HasNormal | RenderItem::HasTangent | RenderItem::HasUV0),
		mCutout(0),
		mBaseColorTexture(),
		mNormalTexture(),
		mMetallicTexture(),
		mRoughnessTexture(),
		mEmissiveTexture(),
		mAmbientOcclusionTexture(),
		mWorldDisplacementTexture()
	{

		mBaseColorTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DWhiteOpaque));
		mNormalTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DNormalMap));
		mMetallicTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DBlackOpaque));
		mRoughnessTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DBlackOpaque));
		mEmissiveTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DBlackOpaque));
		mAmbientOcclusionTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DWhiteOpaque));
		mWorldDisplacementTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DBlackOpaque));
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
			{ "DisplacementAmount", mMaterial.DisplacementAmount }
		};

		bool usedBaseColorTex = mMaterial.TextureStatusMask & (1 << kBaseColor);
		if (usedBaseColorTex)
			data["BaseColorTexture"] = ToString(mBaseColorTexture->GetUuid());

		bool usedMetallicTex = mMaterial.TextureStatusMask & (1 << kMetallic);
		if (usedMetallicTex)
			data["MetallicTexture"] = ToString(mMetallicTexture->GetUuid());

		bool usedRoughnessTex = mMaterial.TextureStatusMask & (1 << kRoughness);
		if (usedRoughnessTex)
			data["RoughnessTexture"] = ToString(mRoughnessTexture->GetUuid());

		bool usedNormalTex = mMaterial.TextureStatusMask & (1 << kNormal);
		if (usedNormalTex)
			data["NormalTexture"] = ToString(mNormalTexture->GetUuid());

		bool usedEmissionTex = mMaterial.TextureStatusMask & (1 << kEmissive);
		if (usedEmissionTex)
			data["EmissionTexture"] = ToString(mEmissiveTexture->GetUuid());

		bool usedAmbientOcclusionTex = mMaterial.TextureStatusMask & (1 << kAmbientOcclusion);
		if (usedAmbientOcclusionTex)
			data["AmbientOcclusionTexture"] = ToString(mAmbientOcclusionTexture->GetUuid());

		bool usedWorldDisplacementTex = mMaterial.TextureStatusMask & (1 << kWorldDisplacement);
		if (usedWorldDisplacementTex)
			data["WorldDisplacementTexture"] = ToString(mWorldDisplacementTexture->GetUuid());

		data["PsoFlags"] = mPsoFlags;

		std::ofstream os(GetPath());
		os << data;
		os.close();
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
		mMaterial.Roughness = data["Roughness"];

		if (data.contains("Metallic"))
			mMaterial.Metallic = data["Metallic"];

		if (data.contains("Opacity"))
			mMaterial.Opacity = data["Opacity"];

		mMaterial.Emissive = XMFLOAT3(data["Emission"].get<std::vector<float>>().data());

		mMaterial.TextureStatusMask = 0;


		mPsoFlags = data.contains("PsoFlags") ? data["PsoFlags"].get<uint16_t>() : 0u;
		mPsoFlags = mPsoFlags | RenderItem::HasPosition | RenderItem::HasNormal | RenderItem::HasTangent | RenderItem::HasUV0;


		if (data.contains("BaseColorTexture"))
		{
			mBaseColorTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RESOURCE::GetResourceHandle(FromString(data["BaseColorTexture"])));

			if (mBaseColorTexture.IsValid())
				mMaterial.TextureStatusMask |= 1 << kBaseColor;

			if (mBaseColorTexture.IsValid() && !mBaseColorTexture->IsLoaded())
				D_RESOURCE_LOADER::LoadResourceAsync(mBaseColorTexture.Get(), nullptr, true);
		}

		if (data.contains("MetallicTexture"))
		{
			mMetallicTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RESOURCE::GetResourceHandle(FromString(data["MetallicTexture"])));

			if (mMetallicTexture.IsValid())
				mMaterial.TextureStatusMask |= 1 << kMetallic;

			if (mMetallicTexture.IsValid() && !mMetallicTexture->IsLoaded())
				D_RESOURCE_LOADER::LoadResourceAsync(mMetallicTexture.Get(), nullptr, true);
		}

		if (data.contains("RoughnessTexture"))
		{
			mRoughnessTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RESOURCE::GetResourceHandle(FromString(data["RoughnessTexture"])));

			if (mRoughnessTexture.IsValid())
				mMaterial.TextureStatusMask |= 1 << kRoughness;

			if (mRoughnessTexture.IsValid() && !mRoughnessTexture->IsLoaded())
				D_RESOURCE_LOADER::LoadResourceAsync(mRoughnessTexture.Get(), nullptr, true);
		}

		if (data.contains("NormalTexture"))
		{
			mNormalTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RESOURCE::GetResourceHandle(FromString(data["NormalTexture"])));

			if (mNormalTexture.IsValid())
				mMaterial.TextureStatusMask |= 1 << kNormal;

			if (mNormalTexture.IsValid() && !mNormalTexture->IsLoaded())
				D_RESOURCE_LOADER::LoadResourceAsync(mNormalTexture.Get(), nullptr, true);
		}

		if (data.contains("EmissionTexture"))
		{
			mEmissiveTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RESOURCE::GetResourceHandle(FromString(data["EmissionTexture"])));

			if (mEmissiveTexture.IsValid())
				mMaterial.TextureStatusMask |= 1 << kEmissive;

			if (mEmissiveTexture.IsValid() && !mEmissiveTexture->IsLoaded())
				D_RESOURCE_LOADER::LoadResourceAsync(mEmissiveTexture.Get(), nullptr, true);
		}

		if (data.contains("AmbientOcclusionTexture"))
		{
			mAmbientOcclusionTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RESOURCE::GetResourceHandle(FromString(data["AmbientOcclusionTexture"])));

			if (mAmbientOcclusionTexture.IsValid())
				mMaterial.TextureStatusMask |= 1 << kAmbientOcclusion;

			if (mAmbientOcclusionTexture.IsValid() && !mAmbientOcclusionTexture->IsLoaded())
				D_RESOURCE_LOADER::LoadResourceAsync(mAmbientOcclusionTexture.Get(), nullptr, true);
		}

		if (data.contains("WorldDisplacementTexture"))
		{
			mWorldDisplacementTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RESOURCE::GetResourceHandle(FromString(data["WorldDisplacementTexture"])));

			if (mWorldDisplacementTexture.IsValid())
			{
				mMaterial.TextureStatusMask |= 1 << kWorldDisplacement;
				mPsoFlags |= RenderItem::HasDisplacement;
			}

			if (mWorldDisplacementTexture.IsValid() && !mWorldDisplacementTexture->IsLoaded())
				D_RESOURCE_LOADER::LoadResourceAsync(mWorldDisplacementTexture.Get(), nullptr, true);
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

	bool MaterialResource::UploadToGpu()
	{
		if (mMaterialConstantsGPU.GetGpuVirtualAddress() == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
		{
			// Initializing Material Constants buffers
			mMaterialConstantsCPU.Create(L"Material Constatns Upload Buffer: " + GetName(), sizeof(MaterialConstants));
			mMaterialConstantsGPU.Create(L"Material Constants GPU Buffer: " + GetName(), 1, sizeof(MaterialConstants), &mMaterial);

			// Update texture regions
			mTexturesHeap = D_RENDERER::AllocateTextureDescriptor(kNumTextures);
			mSamplerTable = D_RENDERER::AllocateSamplerDescriptor(kNumTextures);
		}

		UINT destCount = kNumTextures;
		UINT sourceCounts[kNumTextures] = { 1, 1, 1, 1, 1, 1, 1 };
		D3D12_CPU_DESCRIPTOR_HANDLE initialTextures[kNumTextures]
		{
			mBaseColorTexture->GetTextureData()->GetSRV(),
			mMetallicTexture->GetTextureData()->GetSRV(),
			mRoughnessTexture->GetTextureData()->GetSRV(),
			mAmbientOcclusionTexture->GetTextureData()->GetSRV(),
			mEmissiveTexture->GetTextureData()->GetSRV(),
			mNormalTexture->GetTextureData()->GetSRV(),
			mWorldDisplacementTexture->GetTextureData()->GetSRV()
		};
		D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptors(1, &mTexturesHeap, &destCount, destCount, initialTextures, sourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


		// Loading samplers
		auto incSize = D_GRAPHICS_DEVICE::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		SamplerDesc samplerDesc;
		// Base Color Sampler
		{
			samplerDesc = mBaseColorTexture->GetSamplerDesc();
			samplerDesc.CreateDescriptor(mSamplerTable + incSize * TextureType::kBaseColor);
		}
		// Metallic Sampler
		{
			samplerDesc = mMetallicTexture->GetSamplerDesc();
			samplerDesc.CreateDescriptor(mSamplerTable + incSize * TextureType::kMetallic);
		}
		// Roughness Sampler
		{
			samplerDesc = mRoughnessTexture->GetSamplerDesc();
			samplerDesc.CreateDescriptor(mSamplerTable + incSize * TextureType::kRoughness);
		}
		// Occlussion Sampler
		{
			samplerDesc = mAmbientOcclusionTexture->GetSamplerDesc();
			samplerDesc.CreateDescriptor(mSamplerTable + incSize * TextureType::kAmbientOcclusion);
		}
		// Emissive Sampler
		{
			samplerDesc = mEmissiveTexture->GetSamplerDesc();
			samplerDesc.CreateDescriptor(mSamplerTable + incSize * TextureType::kEmissive);
		}
		// Normal Sampler
		{
			samplerDesc = mNormalTexture->GetSamplerDesc();
			samplerDesc.CreateDescriptor(mSamplerTable + incSize * TextureType::kNormal);
		}
		// World Displacement
		{
			samplerDesc = mWorldDisplacementTexture->GetSamplerDesc();
			samplerDesc.CreateDescriptor(mSamplerTable + incSize * TextureType::kWorldDisplacement);
		}


		// Updating material constnats
		// Mapping upload buffer
		auto matCB = (MaterialConstants*)mMaterialConstantsCPU.Map();
		memcpy(matCB, &mMaterial, sizeof(MaterialConstants));

		mMaterialConstantsCPU.Unmap();

		// Uploading
		auto& context = D_GRAPHICS::CommandContext::Begin(L"Resource Uploader");
		context.TransitionResource(mMaterialConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
		context.GetCommandList()->CopyBufferRegion(mMaterialConstantsGPU.GetResource(), 0, mMaterialConstantsCPU.GetResource(), 0, mMaterialConstantsCPU.GetBufferSize());
		context.TransitionResource(mMaterialConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);
		context.Finish(true);
		return true;
	}

	void MaterialResource::SetTexture(TextureResource* texture, D_RENDERER_RAST::TextureType type)
	{

		// If texuture resource is already set, return
		switch (type)
		{
		case Darius::Renderer::Rasterization::kBaseColor:
			if (mBaseColorTexture == texture)
				return;
			break;
		case Darius::Renderer::Rasterization::kMetallic:
			if (mMetallicTexture == texture)
				return;
			break;
		case Darius::Renderer::Rasterization::kRoughness:
			if (mRoughnessTexture == texture)
				return;
			break;
		case Darius::Renderer::Rasterization::kAmbientOcclusion:
			if (mAmbientOcclusionTexture == texture)
				return;
			break;
		case Darius::Renderer::Rasterization::kEmissive:
			if (mEmissiveTexture == texture)
				return;
			break;
		case Darius::Renderer::Rasterization::kNormal:
			if (mNormalTexture == texture)
				return;
			break;
		case Darius::Renderer::Rasterization::kWorldDisplacement:
			if (mWorldDisplacementTexture == texture)
				return;
			break;
		default:
			return;
		}

		// If texture is null, set to default
		if (texture == nullptr)
		{
			mMaterial.TextureStatusMask &= ~(1 << type);
			switch (type)
			{
			case D_RENDERER_RAST::kBaseColor:
				mBaseColorTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DWhiteOpaque));

				if (!mBaseColorTexture->IsLoaded())
					D_RESOURCE_LOADER::LoadResourceAsync(mBaseColorTexture.Get(), nullptr, true);
				break;
			case D_RENDERER_RAST::kMetallic:
				mMetallicTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DBlackOpaque));

				if (!mMetallicTexture->IsLoaded())
					D_RESOURCE_LOADER::LoadResourceAsync(mMetallicTexture.Get(), nullptr, true);
				break;
			case D_RENDERER_RAST::kRoughness:
				mRoughnessTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DBlackOpaque));

				if (!mRoughnessTexture->IsLoaded())
					D_RESOURCE_LOADER::LoadResourceAsync(mRoughnessTexture.Get(), nullptr, true);
				break;
			case D_RENDERER_RAST::kNormal:
				mNormalTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DNormalMap));

				if (!mNormalTexture->IsLoaded())
					D_RESOURCE_LOADER::LoadResourceAsync(mNormalTexture.Get(), nullptr, true);
				break;
			case D_RENDERER_RAST::kEmissive:
				mEmissiveTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DBlackOpaque));

				if (!mEmissiveTexture->IsLoaded())
					D_RESOURCE_LOADER::LoadResourceAsync(mEmissiveTexture.Get(), nullptr, true);
				break;
			case D_RENDERER_RAST::kAmbientOcclusion:
				mAmbientOcclusionTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DWhiteOpaque));

				if (!mAmbientOcclusionTexture->IsLoaded())
					D_RESOURCE_LOADER::LoadResourceAsync(mAmbientOcclusionTexture.Get(), nullptr, true);
				break;
			case D_RENDERER_RAST::kWorldDisplacement:
				mWorldDisplacementTexture = D_RESOURCE::GetResourceSync<TextureResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DBlackOpaque));

				if (!mWorldDisplacementTexture->IsLoaded())
					D_RESOURCE_LOADER::LoadResourceAsync(mWorldDisplacementTexture.Get(), nullptr, true);
				mPsoFlags &= ~RenderItem::HasDisplacement;
			default:
				return;
			}

			MakeGpuDirty();
			MakeDiskDirty();

			SignalChange();

			return;
		}

		auto device = D_GRAPHICS_DEVICE::GetDevice();
		auto incSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

#define SetTex(name) \
{ \
	m##name##Texture = texture; \
	if(m##name##Texture.IsValid() && !m##name##Texture->IsLoaded()) \
		D_RESOURCE_LOADER::LoadResourceAsync(texture, nullptr, true); \
}

		// Copy texture and sampler descriptor to material descriptor tables

		switch (type)
		{
		case D_RENDERER_RAST::kBaseColor:
			SetTex(BaseColor);
			break;
		case D_RENDERER_RAST::kMetallic:
			SetTex(Metallic);
			break;
		case D_RENDERER_RAST::kRoughness:
			SetTex(Roughness);
			break;
		case D_RENDERER_RAST::kNormal:
			SetTex(Normal);
			break;
		case D_RENDERER_RAST::kEmissive:
			SetTex(Emissive);
			break;
		case D_RENDERER_RAST::kAmbientOcclusion:
			SetTex(AmbientOcclusion);
			break;
		case D_RENDERER_RAST::kWorldDisplacement:
			SetTex(WorldDisplacement);
			mPsoFlags |= RenderItem::HasDisplacement;
			break;
		default:
			break;
		}

		mMaterial.TextureStatusMask |= 1 << type;

		MakeGpuDirty();
		MakeDiskDirty();


		SignalChange();
	}

	// Stand alone texture definitions
#define TexSetterDefinition(type) \
	void MaterialResource::Set##type##Texture(TextureResource* texture) \
	{ \
		SetTexture(texture, k##type);\
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

#define DrawTexture2DHolder(prop, type) \
{ \
	TextureResource* currentTexture = prop.Get(); \
 \
	bool hasTexture = (mMaterial.TextureStatusMask & (1 << k##type)); \
	auto curName = hasTexture ? prop->GetName() : L"<None>"; \
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
		bool nonSel = !prop.IsValid(); \
		if (ImGui::Selectable("<None>", &nonSel)) \
		{ \
			SetTexture(nullptr, k##type); \
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
			DrawTexture2DHolder(mBaseColorTexture, BaseColor);
			ImGui::SameLine();
			ImGui::Text("Albedo");
			if (!HasAlbedoTexture())
			{
				auto value = *reinterpret_cast<Vector4*>(GetAlbedoColor().GetPtr());
				ImGui::TableSetColumnIndex(1);
				float defL[] = D_H_DRAW_DETAILS_MAKE_VEC_PARAM_COLOR;
				if (D_MATH::DrawDetails(value, defL))
				{
					SetAlbedoColor(D_MATH::Color(value));
				}
			}
		}

		// Metallic
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			DrawTexture2DHolder(mMetallicTexture, Metallic);
			ImGui::SameLine();
			ImGui::Text("Metallic");
			if (!HasMetallicTexture())
			{
				float value = GetMetallic();
				ImGui::TableSetColumnIndex(1);
				if (ImGui::SliderFloat("##Metallic", &value, 0.f, 1.f, "% .3f"))
					SetMetallic(value);

			}
		}

		// Roughness
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			DrawTexture2DHolder(mRoughnessTexture, Roughness);
			ImGui::SameLine();
			ImGui::Text("Roughenss");
			if (!HasRoughnessTexture())
			{
				ImGui::TableSetColumnIndex(1);
				float value = GetRoughness();
				if (ImGui::SliderFloat("##Roughness", &value, 0.f, 1.f, "% .3f"))
					SetRoughness(value);
			}
		}

		// Emission
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			DrawTexture2DHolder(mEmissiveTexture, Emissive);
			ImGui::SameLine();
			ImGui::Text("Emission");
			if (!HasEmissiveTexture())
			{
				ImGui::TableSetColumnIndex(1);
				auto value = *reinterpret_cast<Vector3*>(GetEmissiveColor().GetPtr());
				float emS[] = D_H_DRAW_DETAILS_MAKE_VEC_PARAM_COLOR;
				if (D_MATH::DrawDetails(value, emS))
				{
					SetEmissiveColor(D_MATH::Color(value));
				}
			}
		}

		// Opacity
		if(mPsoFlags & RenderItem::AlphaBlend)
		{
			D_H_DETAILS_DRAW_PROPERTY("Opacity");
			float value = GetOpacity();
			if (ImGui::DragFloat("##Opacity", &value, 0.1f, 0.f, 1.f, "%.2f"))
			{
				SetOpacity(value);
			}
		}

		// Normal
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		DrawTexture2DHolder(mNormalTexture, Normal);
		ImGui::SameLine();
		ImGui::Text("Normal");

		// World Displacement
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			DrawTexture2DHolder(mWorldDisplacementTexture, WorldDisplacement);
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

	void MaterialResource::SetEmissiveColor(D_MATH::Color const& value)
	{
		auto valueVector = D_MATH::Vector3(value);
		if (valueVector == mMaterial.Emissive)
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

	bool MaterialResource::AreDependenciesDirty() const
	{
		return
			mBaseColorTexture.IsValidAndGpuDirty() ||
			mNormalTexture.IsValidAndGpuDirty() ||
			mMetallicTexture.IsValidAndGpuDirty() ||
			mRoughnessTexture.IsValidAndGpuDirty() ||
			mEmissiveTexture.IsValidAndGpuDirty() ||
			mWorldDisplacementTexture.IsValidAndGpuDirty() ||
			mAmbientOcclusionTexture.IsValidAndGpuDirty();

	}

}