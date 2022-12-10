#include "Renderer/pch.hpp"
#include "MaterialResource.hpp"

#include "Renderer/RenderDeviceManager.hpp"

#include "Renderer/GraphicsCore.hpp"

#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceLoader.hpp>
#include <Core/Serialization/Json.hpp>
#include <Utils/Common.hpp>

#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>

#include <fstream>

using namespace D_CORE;
using namespace D_SERIALIZATION;
using namespace D_RESOURCE;

namespace Darius::Graphics
{
	D_CH_RESOURCE_DEF(MaterialResource);

	MaterialResource::MaterialResource(Uuid uuid, std::wstring const& path, std::wstring const& name, DResourceId id, bool isDefault) :
		Resource(uuid, path, name, id, isDefault),
		mPsoFlags(0)
	{
		mBaseColorTextureHandle = D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::Texture2DWhiteOpaque);
		mNormalTextureHandle = D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::Texture2DNormalMap);
		mMetallicRoughnessTextureHandle = D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::Texture2DBlackOpaque);
		mEmissiveTextureHandle = D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::Texture2DBlackOpaque);
	}

	void MaterialResource::WriteResourceToFile(D_SERIALIZATION::Json& j) const
	{
		float* defalb = (float*)&mMaterial.DifuseAlbedo;
		float* fren = (float*)&mMaterial.FresnelR0;
		float* ems = (float*)&mMaterial.Emissive;
		Json data = {
			{ "DefuseAlbedo", std::vector<float>(defalb, defalb + 4)},
			{ "FresnelR0", std::vector<float>(fren, fren + 3) },
			{ "Roughness", mMaterial.MetallicRoughness.y },
			{ "Metallic", mMaterial.MetallicRoughness.x },
			{ "Emission", std::vector<float>(ems, ems + 3) }
		};

		bool usedBaseColorTex = mMaterial.TextureStatusMask & (1 << kBaseColor);
		if (usedBaseColorTex)
			data["BaseColorTexture"] = ToString(mBaseColorTexture->GetUuid());

		bool usedRoughnessTex = mMaterial.TextureStatusMask & (1 << kRoughness);
		if (usedRoughnessTex)
			data["MetallicRoughnessTexture"] = ToString(mMetallicRoughnessTexture->GetUuid());

		bool usedNormalTex = mMaterial.TextureStatusMask & (1 << kNormal);
		if (usedNormalTex)
			data["NormalTexture"] = ToString(mNormalTexture->GetUuid());

		bool usedEmissionTex = mMaterial.TextureStatusMask & (1 << kEmissive);
		if (usedEmissionTex)
			data["EmissionTexture"] = ToString(mEmissiveTexture->GetUuid());

		data["PsoFlags"] = mPsoFlags;

		std::ofstream os(GetPath());
		os << data;
	}

	void MaterialResource::ReadResourceFromFile(D_SERIALIZATION::Json const& j)
	{

		Json data;
		std::ifstream is(GetPath());
		is >> data;
		is.close();

		mMaterial.DifuseAlbedo = XMFLOAT4(data["DefuseAlbedo"].get<std::vector<float>>().data());
		mMaterial.FresnelR0 = XMFLOAT3(data["FresnelR0"].get<std::vector<float>>().data());
		mMaterial.MetallicRoughness.y = data["Roughness"];

		if (data.contains("Metallic"))
			mMaterial.MetallicRoughness.x = data["Metallic"];

		mMaterial.Emissive = XMFLOAT3(data["Emission"].get<std::vector<float>>().data());

		mMaterial.TextureStatusMask = 0;

		if (data.contains("BaseColorTexture"))
		{
			mBaseColorTextureHandle = D_RESOURCE::GetResourceHandle(FromString(data["BaseColorTexture"]));
			mMaterial.TextureStatusMask |= 1 << kBaseColor;
		}

		if (data.contains("MetallicRoughnessTexture"))
		{
			mMetallicRoughnessTextureHandle = D_RESOURCE::GetResourceHandle(FromString(data["MetallicRoughnessTexture"]));
			mMaterial.TextureStatusMask |= 1 << kRoughness;
		}

		if (data.contains("NormalTexture"))
		{
			mNormalTextureHandle = D_RESOURCE::GetResourceHandle(FromString(data["NormalTexture"]));
			mMaterial.TextureStatusMask |= 1 << kNormal;
		}

		if (data.contains("EmissionTexture"))
		{
			mEmissiveTextureHandle = D_RESOURCE::GetResourceHandle(FromString(data["EmissionTexture"]));
			mMaterial.TextureStatusMask |= 1 << kEmissive;
		}
		mPsoFlags = data.contains("PsoFlags") ? data["PsoFlags"].get<uint16_t>() : 0u;
	}

	bool MaterialResource::UploadToGpu(void* ctx)
	{
		D_GRAPHICS::GraphicsContext& context = *reinterpret_cast<D_GRAPHICS::GraphicsContext*>(ctx);
		if (mMaterialConstantsGPU.GetGpuVirtualAddress() == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
		{
			// Load resources
			mBaseColorTexture = D_RESOURCE::GetResource<TextureResource>(mBaseColorTextureHandle, *this);
			mNormalTexture = D_RESOURCE::GetResource<TextureResource>(mNormalTextureHandle, *this);
			mMetallicRoughnessTexture = D_RESOURCE::GetResource<TextureResource>(mMetallicRoughnessTextureHandle, *this);
			mEmissiveTexture = D_RESOURCE::GetResource<TextureResource>(mEmissiveTextureHandle, *this);

			// Initializing Material Constants buffers
			for (size_t i = 0; i < D_RENDERER_FRAME_RESOUCE::gNumFrameResources; i++)
			{
				mMaterialConstantsCPU[i].Create(L"Material Constatns Upload Buffer: " + GetName(), sizeof(MaterialConstants));
			}
			mMaterialConstantsGPU.Create(L"Material Constants GPU Buffer: " + GetName(), 1, sizeof(MaterialConstants), &mMaterial);

			// Update texture regions
			mTexturesHeap = D_RENDERER::AllocateTextureDescriptor(kNumTextures);

			UINT destCount = kNumTextures;
			UINT sourceCounts[kNumTextures] = { 1, 1, 1, 1, 1 };
			D3D12_CPU_DESCRIPTOR_HANDLE initialTextures[kNumTextures]
			{
				mBaseColorTexture->GetTextureData()->GetSRV(),
				mMetallicRoughnessTexture->GetTextureData()->GetSRV(),
				mMetallicRoughnessTexture->GetTextureData()->GetSRV(),
				mEmissiveTexture->GetTextureData()->GetSRV(),
				mNormalTexture->GetTextureData()->GetSRV()
			};
			D_RENDERER_DEVICE::GetDevice()->CopyDescriptors(1, &mTexturesHeap, &destCount, destCount, initialTextures, sourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			return true;
		}

		// Updating material constnats
		// Mapping upload buffer
		auto& currentMatUploadBuff = mMaterialConstantsCPU[D_RENDERER_DEVICE::GetCurrentResourceIndex()];
		auto matCB = (MaterialConstants*)currentMatUploadBuff.Map();
		memcpy(matCB, &mMaterial, sizeof(MaterialConstants));

		currentMatUploadBuff.Unmap();

		// Uploading
		context.TransitionResource(mMaterialConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
		context.GetCommandList()->CopyBufferRegion(mMaterialConstantsGPU.GetResource(), 0, currentMatUploadBuff.GetResource(), 0, currentMatUploadBuff.GetBufferSize());
		context.TransitionResource(mMaterialConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);
		return true;
	}

	void MaterialResource::SetTexture(ResourceHandle textureHandle, D_RENDERER::TextureType type)
	{

		if (textureHandle.Type != TextureResource::GetResourceType())
		{
			mMaterial.TextureStatusMask &= ~(1 << type);
			switch (type)
			{
			case Darius::Renderer::kBaseColor:
				mBaseColorTextureHandle = EmptyResourceHandle;
				mBaseColorTexture.Unref();
				break;
			case Darius::Renderer::kRoughness:
				mMetallicRoughnessTextureHandle = EmptyResourceHandle;
				mMetallicRoughnessTexture.Unref();
				break;
			case Darius::Renderer::kNormal:
				mNormalTextureHandle = EmptyResourceHandle;
				mNormalTexture.Unref();
				break;
			case Darius::Renderer::kEmissive:
				mEmissiveTextureHandle = EmptyResourceHandle;
				mEmissiveTexture.Unref();
				break;
			case Darius::Renderer::kOcculusion:
			default:
				return;
			}

			MakeGpuDirty();
			MakeDiskDirty();

			return;
		}

		auto device = D_RENDERER_DEVICE::GetDevice();
		auto incSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

#define SetTex(name) \
m##name##TextureHandle = textureHandle; \
m##name##Texture = D_RESOURCE::GetResource<TextureResource>(textureHandle, *this); \
device->CopyDescriptorsSimple(1, mTexturesHeap + type * incSize, m##name##Texture->GetTextureData()->GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		auto& context = D_GRAPHICS::GraphicsContext::Begin(L"Set Material Texture");

		switch (type)
		{
		case Darius::Renderer::kBaseColor:
			SetTex(BaseColor);
			break;
		case Darius::Renderer::kRoughness:
			SetTex(MetallicRoughness);
			break;
		case Darius::Renderer::kNormal:
			SetTex(Normal);
			break;
		case Darius::Renderer::kEmissive:
			SetTex(Emissive);
			break;
		case Darius::Renderer::kOcculusion:
		default:
			return;
		}

		mMaterial.TextureStatusMask |= 1 << type;
		MakeGpuDirty();
		MakeDiskDirty();

	}


#ifdef _D_EDITOR
	bool MaterialResource::DrawDetails(float params[])
	{
#define DrawTexture2DHolder(prop, type) \
{ \
	TextureResource* currentTexture = prop.Get(); \
 \
	bool hasTexture = (mMaterial.TextureStatusMask & (1 << type)); \
	auto curName = hasTexture ? prop->GetName() : L"<None>"; \
	const char* selectButtonName = hasTexture ? ICON_FA_IMAGE "##" #type : ICON_FA_SQUARE "##" #type; \
	if (ImGui::Button(selectButtonName)) \
	{ \
		\
		ImGui::OpenPopup("Select " #type); \
		D_LOG_DEBUG("OPEN " #type); \
	} \
		 \
	if (ImGui::BeginPopup("Select " #type)) \
	{ \
		bool nonSel = !prop.IsValid(); \
		if (ImGui::Selectable("<None>", &nonSel)) \
		{ \
			SetTexture(EmptyResourceHandle, type); \
			valueChanged = true; \
		} \
			 \
		auto meshes = D_RESOURCE::GetResourcePreviews(TextureResource::GetResourceType()); \
		int idx = 0; \
		for (auto prev : meshes) \
		{ \
			bool selected = currentTexture && prev.Handle.Id == currentTexture->GetId() && prev.Handle.Type == currentTexture->GetType(); \
 \
			auto name = STR_WSTR(prev.Name); \
			ImGui::PushID((name + std::to_string(idx)).c_str()); \
			if (ImGui::Selectable(name.c_str(), &selected)) \
			{ \
				SetTexture(prev.Handle, type); \
				valueChanged = true; \
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
		bool valueChanged = false;

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		D_H_DETAILS_DRAW_PROPERTY("Shader Type");
		if (ImGui::Button(mPsoFlags & RenderItem::AlphaBlend ? "Transparent" : "Opaque", ImVec2(-1, 0)))
		{
			ImGui::OpenPopup("##ShaderTypeSelecionPopup");
			valueChanged = true;
		}
		if (ImGui::BeginPopupContextItem("##ShaderTypeSelecionPopup", ImGuiPopupFlags_NoOpenOverExistingPopup))
		{
			if (ImGui::Selectable("Opaque"))
			{
				mPsoFlags &= ~RenderItem::AlphaBlend;
			}

			if (ImGui::Selectable("Transparent"))
			{
				mPsoFlags |= RenderItem::AlphaBlend;
			}
			ImGui::EndPopup();
		}

		// Diffuse
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		DrawTexture2DHolder(mBaseColorTexture, kBaseColor);
		ImGui::SameLine();
		ImGui::Text("Diffuse");
		if (!(mMaterial.TextureStatusMask & (1 << kBaseColor)))
		{
			ImGui::TableSetColumnIndex(1);
			float defL[] = D_H_DRAW_DETAILS_MAKE_VEC_PARAM_COLOR;
			if (D_MATH::DrawDetails(*(Vector4*)&mMaterial.DifuseAlbedo, defL))
			{
				valueChanged = true;
			}
		}

		// Roughness
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		DrawTexture2DHolder(mMetallicRoughnessTexture, kRoughness);
		ImGui::SameLine();
		ImGui::Text("Roughness");
		if (!(mMaterial.TextureStatusMask & (1 << kRoughness)))
		{
			ImGui::TableSetColumnIndex(1);
			float defS[] = { 1.f, 0.f };
			if (ImGui::DragFloat("##Roughness", &mMaterial.MetallicRoughness.y, 0.001f, 0.f, 1.f, "% .3f"))
			{
				valueChanged = true;
			}

			D_H_DETAILS_DRAW_PROPERTY("Metallic");
			if (ImGui::DragFloat("##Metallic", &mMaterial.MetallicRoughness.x, 0.001f, 0.f, 1.f, "% .3f"))
			{
				valueChanged = true;
			}
		}

		// Emission
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		DrawTexture2DHolder(mEmissiveTexture, kEmissive);
		ImGui::SameLine();
		ImGui::Text("Emission");
		if (!(mMaterial.TextureStatusMask & (1 << kEmissive)))
		{
			ImGui::TableSetColumnIndex(1);
			float emS[] = D_H_DRAW_DETAILS_MAKE_VEC_PARAM_COLOR;
			if (D_MATH::DrawDetails(*(Vector3*)&mMaterial.Emissive, emS))
			{
				valueChanged = true;
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
#endif // _D_EDITOR

}