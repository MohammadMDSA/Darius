#include "Renderer/pch.hpp"
#include "MaterialResource.hpp"
#include "ResourceManager/ResourceManager.hpp"

#include <Core/Serialization/Json.hpp>
#include <Renderer/RenderDeviceManager.hpp>
#include <Utils/Common.hpp>

#include <imgui.h>
#include <nlohmann/json.hpp>

#include <fstream>

using namespace D_CORE;
using namespace D_SERIALIZATION;

namespace Darius::ResourceManager
{
	MaterialResource::MaterialResource(Uuid uuid, std::wstring const& path, DResourceId id, bool isDefault) :
		Resource(uuid, path, id, isDefault)
	{
		mBaseColorTextureHandle = D_RESOURCE::GetDefaultResource(D_RESOURCE::DefaultResource::Texture2DWhiteOpaque);
		mNormalTextureHandle = D_RESOURCE::GetDefaultResource(D_RESOURCE::DefaultResource::Texture2DNormalMap);
		mRoughnessTextureHandle = D_RESOURCE::GetDefaultResource(D_RESOURCE::DefaultResource::Texture2DBlackOpaque);
	}

	bool MaterialResource::SuppoertsExtension(std::wstring ext)
	{
		if (ext == L".mat")
			return true;
		return false;
	}

	void MaterialResource::WriteResourceToFile() const
	{
		float* defalb = (float*)&mMaterial.DifuseAlbedo;
		float* fren = (float*)&mMaterial.FresnelR0;
		Json data = {
			{ "DefuseAlbedo", std::vector<float>(defalb, defalb + 4)},
			{ "FresnelR0", std::vector<float>(fren, fren + 3) },
			{ "Roughness", mMaterial.Roughness }
		};

		bool usedBaseColorTex = mMaterial.TextureStatusMask & (1 << kBaseColor);
		if (usedBaseColorTex)
			data["BaseColorTexture"] = ToString(mBaseColorTexture->GetUuid());

		bool usedRoughnessTex = mMaterial.TextureStatusMask & (1 << kRoughness);
		if (usedRoughnessTex)
			data["RoughnessTexture"] = ToString(mRoughnessTexture->GetUuid());

		bool usedNormalTex = mMaterial.TextureStatusMask & (1 << kNormal);
		if (usedNormalTex)
			data["NormalTexture"] = ToString(mNormalTexture->GetUuid());

		std::ofstream os(GetPath());
		os << data;
	}

	void MaterialResource::ReadResourceFromFile()
	{

		Json data;
		std::ifstream is(GetPath());
		is >> data;
		is.close();

		mMaterial.DifuseAlbedo = XMFLOAT4(data["DefuseAlbedo"].get<std::vector<float>>().data());
		mMaterial.FresnelR0 = XMFLOAT3(data["FresnelR0"].get<std::vector<float>>().data());
		mMaterial.Roughness = data["Roughness"];
		mMaterial.TextureStatusMask = 0;

		if (data.contains("BaseColorTexture"))
		{
			mBaseColorTextureHandle = D_RESOURCE::GetResourceHandle(FromString(data["BaseColorTexture"]));
			mMaterial.TextureStatusMask |= 1 << kBaseColor;
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
	}

	bool MaterialResource::UploadToGpu(D_GRAPHICS::GraphicsContext& context)
	{
		if (mMaterialConstantsGPU.GetGpuVirtualAddress() == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
		{
			// Load resources
			mBaseColorTexture = D_RESOURCE::GetResource<Texture2DResource>(mBaseColorTextureHandle, *this);
			mNormalTexture = D_RESOURCE::GetResource<Texture2DResource>(mNormalTextureHandle, *this);
			mRoughnessTexture = D_RESOURCE::GetResource<Texture2DResource>(mRoughnessTextureHandle, *this);

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
				mRoughnessTexture->GetTextureData()->GetSRV(),
				mRoughnessTexture->GetTextureData()->GetSRV(),
				mRoughnessTexture->GetTextureData()->GetSRV(),
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

		if (textureHandle.Type != ResourceType::Texture2D)
		{
			mMaterial.TextureStatusMask &= ~(1 << type);
			switch (type)
			{
			case Darius::Renderer::kBaseColor:
				mBaseColorTextureHandle = EmptyHandle;
				mBaseColorTexture.Unref();
				break;
			case Darius::Renderer::kRoughness:
				mRoughnessTextureHandle = EmptyHandle;
				mRoughnessTexture.Unref();
				break;
			case Darius::Renderer::kNormal:
				mNormalTextureHandle = EmptyHandle;
				mNormalTexture.Unref();
				break;
			case Darius::Renderer::kOcculusion:
			case Darius::Renderer::kEmissive:
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
m##name##Texture = D_RESOURCE::GetResource<Texture2DResource>(textureHandle, *this); \
device->CopyDescriptorsSimple(1, mTexturesHeap + type * incSize, m##name##Texture->GetTextureData()->GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		auto& context = D_GRAPHICS::GraphicsContext::Begin(L"Set Material Texture");

		switch (type)
		{
		case Darius::Renderer::kBaseColor:
			SetTex(BaseColor);
			break;
		case Darius::Renderer::kRoughness:
			SetTex(Roughness);
			break;
		case Darius::Renderer::kNormal:
			SetTex(Normal);
			break;
		case Darius::Renderer::kOcculusion:
		case Darius::Renderer::kEmissive:
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
		if (!GetLoaded())
		{
			this->ReadResourceFromFile();
			auto& context = D_GRAPHICS::GraphicsContext::Begin(L"Force gpu update on detail");
			this->UpdateGPU(context);
			context.Finish(true);
		}

#define DrawTexture2DHolder(prop, type) \
{ \
	Texture2DResource* currentTexture = prop.Get(); \
 \
	auto curName = (mMaterial.TextureStatusMask & (1 << type)) ? prop->GetName() : L"<None>"; \
	if (ImGui::Button(STR_WSTR(curName).c_str())) \
	{ \
		\
		ImGui::OpenPopup("Select " #type); \
	} \
		 \
	if (ImGui::BeginPopup("Select " #type)) \
	{ \
		bool nonSel = !prop.IsValid(); \
		if (ImGui::Selectable("<None>", &nonSel)) \
		{ \
			SetTexture(EmptyHandle, type); \
			valueChanged = true; \
		} \
			 \
		auto meshes = D_RESOURCE::GetResourcePreviews(D_RESOURCE::ResourceType::Texture2D); \
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


			if (ImGui::BeginTable("mat editor", 2, ImGuiTableFlags_BordersInnerV))
			{
				ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 100.f);
				ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);


				// Diffuse
				if (!(mMaterial.TextureStatusMask & (1 << kBaseColor)))
				{

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Diffuse Color");

					ImGui::TableSetColumnIndex(1);
					float defL[] = { 0.f, 1.f };
					if (D_MATH::DrawDetails(*(Vector4*)&mMaterial.DifuseAlbedo, defL))
					{
						valueChanged = true;
					}
				}

				// Fresnel
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Fresnel");

				ImGui::TableSetColumnIndex(1);
				float defR[] = { 0.f, 1.f };
				if (D_MATH::DrawDetails(*(Vector3*)&mMaterial.FresnelR0, defR))
				{
					valueChanged = true;
				}

				// Roughness
				if (!(mMaterial.TextureStatusMask & (1 << kRoughness)))
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Roughness");

					ImGui::TableSetColumnIndex(1);
					float defS[] = { 1.f, 0.f };
					if (ImGui::DragFloat("##X", &mMaterial.Roughness, 0.01f, 0.f, 1.f, "% .3f"))
					{
						valueChanged = true;
					}
				}

				ImGui::EndTable();
			}

			// Texture selection
			{
				// Base Color
				DrawTexture2DHolder(mBaseColorTexture, kBaseColor);

				// Roughness
				DrawTexture2DHolder(mRoughnessTexture, kRoughness);
			}


			return valueChanged;
		}
	}
#endif // _D_EDITOR

}