#include "Renderer/pch.hpp"
#include "MaterialResource.hpp"

#include "Renderer/GraphicsDeviceManager.hpp"

#include "Renderer/GraphicsCore.hpp"

#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceLoader.hpp>
#include <Core/Serialization/Json.hpp>
#include <Utils/Common.hpp>
#include <Utils/DragDropPayload.hpp>

#ifdef _D_EDITOR
#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>
#endif

#include <fstream>

#include <MaterialResource.sgenerated.hpp>

using namespace D_CORE;
using namespace D_GRAPHICS_UTILS;
using namespace D_MATH;
using namespace D_SERIALIZATION;
using namespace D_RENDERER;
using namespace D_RENDERER_FRAME_RESOURCE;
using namespace D_RESOURCE;
using namespace DirectX;

inline uint32_t floatToHalf(float f)
{
	const float kF32toF16 = (1.0 / (1ull << 56)) * (1.0 / (1ull << 56)); // 2^-112
	union { float f; uint32_t u; } x;
	x.f = D_MATH::Clamp(f, 0.0f, 1.0f) * kF32toF16;
	return x.u >> 13;
}

namespace Darius::Graphics
{
	D_CH_RESOURCE_DEF(MaterialResource);

	MaterialResource::MaterialResource(Uuid uuid, std::wstring const& path, std::wstring const& name, DResourceId id, bool isDefault) :
		Resource(uuid, path, name, id, isDefault),
		mPsoFlags(RenderItem::HasPosition | RenderItem::HasNormal | RenderItem::HasTangent | RenderItem::HasUV0),
		mCutout(0),
		mBaseColorTexture(GetAsCountedOwner()),
		mNormalTexture(GetAsCountedOwner()),
		mMetallicTexture(GetAsCountedOwner()),
		mRoughnessTexture(GetAsCountedOwner()),
		mEmissiveTexture(GetAsCountedOwner()),
		mAmbientOcclusionTexture(GetAsCountedOwner())
	{
		auto callback = [&]() { MakeGpuDirty(); };
		mBaseColorTexture.SetChangeCallback(callback);
		mNormalTexture.SetChangeCallback(callback);
		mMetallicTexture.SetChangeCallback(callback);
		mRoughnessTexture.SetChangeCallback(callback);
		mEmissiveTexture.SetChangeCallback(callback);
		mAmbientOcclusionTexture.SetChangeCallback(callback);
		mBaseColorTextureHandle = D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::Texture2DWhiteOpaque);
		mNormalTextureHandle = D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::Texture2DNormalMap);
		mMetallicTextureHandle = D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::Texture2DBlackOpaque);
		mRoughnessTextureHandle = D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::Texture2DBlackOpaque);
		mEmissiveTextureHandle = D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::Texture2DBlackOpaque);
		mAmbientOcclusionTextureHandle = D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::Texture2DWhiteOpaque);
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
			{ "AlphaCutout", mCutout }
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
		mMaterial.Roughness = data["Roughness"];

		if (data.contains("Metallic"))
			mMaterial.Metallic = data["Metallic"];

		mMaterial.Emissive = XMFLOAT3(data["Emission"].get<std::vector<float>>().data());

		mMaterial.TextureStatusMask = 0;

		if (data.contains("BaseColorTexture"))
		{
			mBaseColorTextureHandle = D_RESOURCE::GetResourceHandle(FromString(data["BaseColorTexture"]));
			mMaterial.TextureStatusMask |= 1 << kBaseColor;
		}

		if (data.contains("MetallicTexture"))
		{
			mMetallicTextureHandle = D_RESOURCE::GetResourceHandle(FromString(data["MetallicTexture"]));
			mMaterial.TextureStatusMask |= 1 << kMetallic;
		}

		if (data.contains("RoughnessTexture"))
		{
			mRoughnessTextureHandle = D_RESOURCE::GetResourceHandle(FromString(data["RoughnessTexture"]));
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

		if (data.contains("AmbientOcclusionTexture"))
		{
			mAmbientOcclusionTextureHandle = D_RESOURCE::GetResourceHandle(FromString(data["AmbientOcclusionTexture"]));
			mMaterial.TextureStatusMask |= 1 << kAmbientOcclusion;
		}

		if (data.contains("AlphaCutout"))
		{
			mCutout = data["AlphaCutout"];
			mMaterial.AlphaCutout = floatToHalf(mCutout);
		}

		mPsoFlags = data.contains("PsoFlags") ? data["PsoFlags"].get<uint16_t>() : 0u;
		mPsoFlags = mPsoFlags | RenderItem::HasPosition | RenderItem::HasNormal | RenderItem::HasTangent | RenderItem::HasUV0;

	}

	bool MaterialResource::UploadToGpu()
	{
		if (mMaterialConstantsGPU.GetGpuVirtualAddress() == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
		{


			// Initializing Material Constants buffers
			for (size_t i = 0; i < D_RENDERER_FRAME_RESOURCE::gNumFrameResources; i++)
			{
				mMaterialConstantsCPU[i].Create(L"Material Constatns Upload Buffer: " + GetName(), sizeof(MaterialConstants));
			}
			mMaterialConstantsGPU.Create(L"Material Constants GPU Buffer: " + GetName(), 1, sizeof(MaterialConstants), &mMaterial);

			// Update texture regions
			mTexturesHeap = D_RENDERER::AllocateTextureDescriptor(kNumTextures);
			mSamplerTable = D_RENDERER::AllocateSamplerDescriptor(kNumTextures);
		}
		D_CORE::CountedOwner countedOwner = *this;
		countedOwner.ChangeCallback = [&]()
		{
			MakeGpuDirty();
		};

		// Load resources
		mBaseColorTexture = D_RESOURCE::GetResource<TextureResource>(mBaseColorTextureHandle);
		mNormalTexture = D_RESOURCE::GetResource<TextureResource>(mNormalTextureHandle);
		mMetallicTexture = D_RESOURCE::GetResource<TextureResource>(mMetallicTextureHandle);
		mRoughnessTexture = D_RESOURCE::GetResource<TextureResource>(mRoughnessTextureHandle);
		mEmissiveTexture = D_RESOURCE::GetResource<TextureResource>(mEmissiveTextureHandle);
		mAmbientOcclusionTexture = D_RESOURCE::GetResource<TextureResource>(mAmbientOcclusionTextureHandle);

		UINT destCount = kNumTextures;
		UINT sourceCounts[kNumTextures] = { 1, 1, 1, 1, 1, 1 };
		D3D12_CPU_DESCRIPTOR_HANDLE initialTextures[kNumTextures]
		{
			mBaseColorTexture->GetTextureData()->GetSRV(),
			mMetallicTexture->GetTextureData()->GetSRV(),
			mRoughnessTexture->GetTextureData()->GetSRV(),
			mAmbientOcclusionTexture->GetTextureData()->GetSRV(),
			mEmissiveTexture->GetTextureData()->GetSRV(),
			mNormalTexture->GetTextureData()->GetSRV()
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


		// Updating material constnats
		// Mapping upload buffer
		auto& currentMatUploadBuff = mMaterialConstantsCPU[D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex()];
		auto matCB = (MaterialConstants*)currentMatUploadBuff.Map();
		memcpy(matCB, &mMaterial, sizeof(MaterialConstants));

		currentMatUploadBuff.Unmap();

		// Uploading
		auto& context = D_GRAPHICS::CommandContext::Begin(L"Resource Uploader");
		context.TransitionResource(mMaterialConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
		context.GetCommandList()->CopyBufferRegion(mMaterialConstantsGPU.GetResource(), 0, currentMatUploadBuff.GetResource(), 0, currentMatUploadBuff.GetBufferSize());
		context.TransitionResource(mMaterialConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);
		context.Finish(true);
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
				mBaseColorTextureHandle = D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::Texture2DWhiteOpaque);
				break;
			case Darius::Renderer::kMetallic:
				mMetallicTextureHandle = D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::Texture2DNormalMap);
				break;
			case Darius::Renderer::kRoughness:
				mRoughnessTextureHandle = D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::Texture2DBlackOpaque);
				break;
			case Darius::Renderer::kNormal:
				mNormalTextureHandle = D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::Texture2DBlackOpaque);
				break;
			case Darius::Renderer::kEmissive:
				mEmissiveTextureHandle = D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::Texture2DBlackOpaque);
				break;
			case Darius::Renderer::kAmbientOcclusion:
				mAmbientOcclusionTextureHandle = D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::Texture2DWhiteOpaque);
				break;
			default:
				return;
			}
			
			MakeGpuDirty();
			MakeDiskDirty();

			return;
		}

		auto device = D_GRAPHICS_DEVICE::GetDevice();
		auto incSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

#define SetTex(name) \
{ \
	m##name##TextureHandle = textureHandle; \
	m##name##Texture = D_RESOURCE::GetResource<TextureResource>(textureHandle); \
	device->CopyDescriptorsSimple(1, mTexturesHeap + type * incSize, m##name##Texture->GetTextureData()->GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); \
	SamplerDesc sd = m##name##Texture->GetSamplerDesc(); \
	sd.CreateDescriptor(mSamplerTable + incSize * type); \
}

		auto& context = D_GRAPHICS::GraphicsContext::Begin(L"Set Material Texture");

		switch (type)
		{
		case Darius::Renderer::kBaseColor:
			SetTex(BaseColor);
			break;
		case Darius::Renderer::kMetallic:
			SetTex(Metallic);
			break;
		case Darius::Renderer::kRoughness:
			SetTex(Roughness);
			break;
		case Darius::Renderer::kNormal:
			SetTex(Normal);
			break;
		case Darius::Renderer::kEmissive:
			SetTex(Emissive);
			break;
		case Darius::Renderer::kAmbientOcclusion:
			SetTex(AmbientOcclusion);
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
			SetTexture(EmptyResourceHandle, k##type); \
			valueChanged = true; \
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
				SetTexture(prev.Handle, k##type); \
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

		// Diffuse
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		DrawTexture2DHolder(mBaseColorTexture, BaseColor);
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

		// Metallic
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		DrawTexture2DHolder(mMetallicTexture, Metallic);
		ImGui::SameLine();
		ImGui::Text("Metallic");
		if (!(mMaterial.TextureStatusMask & (1 << kMetallic)))
		{
			ImGui::TableSetColumnIndex(1);
			valueChanged |= ImGui::SliderFloat("##Metallic", &mMaterial.Metallic, 0.f, 1.f, "% .3f");
		}

		// Roughness
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		DrawTexture2DHolder(mRoughnessTexture, Roughness);
		ImGui::SameLine();
		ImGui::Text("Roughenss");
		if (!(mMaterial.TextureStatusMask & (1 << kRoughness)))
		{
			ImGui::TableSetColumnIndex(1);
			valueChanged |= ImGui::SliderFloat("##Roughness", &mMaterial.Roughness, 0.f, 1.f, "% .3f");
		}

		// Emission
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		DrawTexture2DHolder(mEmissiveTexture, Emissive);
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

		// Normal
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		DrawTexture2DHolder(mNormalTexture, Normal);
		ImGui::SameLine();
		ImGui::Text("Normal");


		// Two sided
		{
			bool val = mPsoFlags & RenderItem::TwoSided;
			D_H_DETAILS_DRAW_PROPERTY("Two Sided");

			if (ImGui::Checkbox("##TwoSided", &val))
			{
				valueChanged = true;
				if (val)
					mPsoFlags |= RenderItem::TwoSided;
				else
					mPsoFlags &= ~RenderItem::TwoSided;
			}
		}

		// Alpha test cutout
		if (mPsoFlags & RenderItem::AlphaTest)
		{
			D_H_DETAILS_DRAW_PROPERTY("Alpha Cutout");
			if (ImGui::SliderFloat("##AlphaCutout", &mCutout, 0, 1, "%.2f"))
			{
				valueChanged = true;
				mMaterial.AlphaCutout = floatToHalf(mCutout);
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

	bool MaterialResource::IsDirtyGPU() const
	{
		return Resource::IsDirtyGPU() ||
			(mBaseColorTexture.IsValid() && mBaseColorTexture->IsDirtyGPU()) ||
			(mNormalTexture.IsValid() && mNormalTexture->IsDirtyGPU()) ||
			(mMetallicTexture.IsValid() && mMetallicTexture->IsDirtyGPU()) ||
			(mRoughnessTexture.IsValid() && mRoughnessTexture->IsDirtyGPU()) ||
			(mEmissiveTexture.IsValid() && mEmissiveTexture->IsDirtyGPU()) ||
			(mAmbientOcclusionTexture.IsValid() && mAmbientOcclusionTexture->IsDirtyGPU());
	}

}