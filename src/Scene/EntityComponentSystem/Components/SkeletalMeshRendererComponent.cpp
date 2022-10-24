#include "Scene/pch.hpp"
#include "SkeletalMeshRendererComponent.hpp"
#include "Scene/Utils/DetailsDrawer.hpp"

#include <ResourceManager/ResourceManager.hpp>

#include <imgui.h>

using namespace D_SERIALIZATION;
using namespace D_RESOURCE;
using namespace D_RENDERER_FRAME_RESOUCE;

namespace Darius::Scene::ECS::Components
{
	D_H_COMP_DEF(SkeletalMeshRendererComponent);


	SkeletalMeshRendererComponent::SkeletalMeshRendererComponent() :
		ComponentBase(),
		mPsoFlags(0)
	{
	}

	SkeletalMeshRendererComponent::SkeletalMeshRendererComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid),
		mPsoFlags(0)
	{
	}

	void SkeletalMeshRendererComponent::Start()
	{

		// Initializing Mesh Constants buffers
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOUCE::gNumFrameResources; i++)
		{
			mMeshConstantsCPU[i].Create(L"Mesh Constant Upload Buffer", sizeof(MeshConstants));
		}
		mMeshConstantsGPU.Create(L"Mesh Constant GPU Buffer", 1, sizeof(MeshConstants));

		if (!mMaterialResource.IsValid())
			_SetMaterial(D_RESOURCE::GetDefaultResource(DefaultResource::Material));
	}

	RenderItem SkeletalMeshRendererComponent::GetRenderItem()
	{
		auto result = RenderItem();
		const Mesh* mesh = mMeshResource.Get()->GetMeshData();
		result.BaseVertexLocation = mesh->mDraw[0].BaseVertexLocation;
		result.IndexCount = mesh->mNumTotalIndices;
		result.StartIndexLocation = mesh->mDraw[0].StartIndexLocation;
		result.Mesh = mesh;
		result.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		result.MeshCBV = GetConstantsAddress();
		result.Material.MaterialCBV = *mMaterialResource.Get();
		result.Material.MaterialSRV = mMaterialResource->GetTexturesHandle();
		result.PsoType = D_RENDERER::SkinnedOpaquePso;
		result.PsoFlags = mPsoFlags | mMaterialResource->GetPsoFlags();
		return result;
	}

#ifdef _D_EDITOR
	bool SkeletalMeshRendererComponent::DrawDetails(float params[])
	{
		auto changeValue = false;

		if (ImGui::BeginTable("mesh editor", 2, ImGuiTableFlags_BordersInnerV))
		{
			// Shader type
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 100.f);
			ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Mesh");
			ImGui::TableSetColumnIndex(1);
			// Mesh selection
			{
				MeshResource* currentMesh = mMeshResource.Get();

				if (ImGui::Button("Select"))
				{
					ImGui::OpenPopup("Select Res");
				}

				if (ImGui::BeginPopup("Select Res"))
				{
					auto meshes = D_RESOURCE::GetResourcePreviews(SkeletalMeshResource::GetResourceType());
					int idx = 0;
					for (auto prev : meshes)
					{
						bool selected = currentMesh && prev.Handle.Id == currentMesh->GetId() && prev.Handle.Type == currentMesh->GetType();

						auto name = STR_WSTR(prev.Name);
						ImGui::PushID((name + std::to_string(idx)).c_str());
						if (ImGui::Selectable(name.c_str(), &selected))
						{
							SetMesh(prev.Handle);
							changeValue = true;
						}
						ImGui::PopID();

						idx++;
					}

					ImGui::EndPopup();
				}
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Material");
			ImGui::TableSetColumnIndex(1);
			// Material selection
			{
				MaterialResource* currentMaterial = mMaterialResource.Get();

				if (ImGui::Button("Select"))
				{
					ImGui::OpenPopup("Select Mat");
				}

				if (ImGui::BeginPopup("Select Mat"))
				{
					auto meshes = D_RESOURCE::GetResourcePreviews(MaterialResource::GetResourceType());
					int idx = 0;
					for (auto prev : meshes)
					{
						bool selected = currentMaterial && prev.Handle.Id == currentMaterial->GetId() && prev.Handle.Type == currentMaterial->GetType();

						auto name = STR_WSTR(prev.Name);
						ImGui::PushID((name + std::to_string(idx)).c_str());
						if (ImGui::Selectable(name.c_str(), &selected))
						{
							SetMaterial(prev.Handle);
							changeValue = true;
						}
						ImGui::PopID();

						idx++;
					}

					ImGui::EndPopup();
				}

				ImGui::EndTable();
			}
		}

		return changeValue;

	}
#endif


	void SkeletalMeshRendererComponent::SetMesh(ResourceHandle handle)
	{
		mChangeSignal();
		_SetMesh(handle);
	}

	void SkeletalMeshRendererComponent::SetMaterial(ResourceHandle handle)
	{
		mChangeSignal();
		_SetMaterial(handle);
	}

	void SkeletalMeshRendererComponent::_SetMesh(ResourceHandle handle)
	{
		mMeshResource = D_RESOURCE::GetResource<SkeletalMeshResource>(handle, *GetGameObject());
	}

	void SkeletalMeshRendererComponent::_SetMaterial(ResourceHandle handle)
	{
		mMaterialResource = D_RESOURCE::GetResource<MaterialResource>(handle, *GetGameObject());
	}

	void SkeletalMeshRendererComponent::Serialize(Json& j) const
	{
		D_CORE::to_json(j["Material"], mMaterialResource.Get()->GetUuid());
		D_CORE::to_json(j["Mesh"], mMeshResource.Get()->GetUuid());
	}

	void SkeletalMeshRendererComponent::Deserialize(Json const& j)
	{
		auto go = GetGameObject();

		// Loading material
		Uuid materialUuid;
		D_CORE::from_json(j["Material"], materialUuid);
		mMaterialResource = D_RESOURCE::GetResource<MaterialResource>(materialUuid, *go);

		// Loading mesh
		Uuid meshUuid;
		D_CORE::from_json(j["Mesh"], meshUuid);
		mMeshResource = D_RESOURCE::GetResource<SkeletalMeshResource>(meshUuid, *go);
	}

	void SkeletalMeshRendererComponent::Update(float dt)
	{
		if (!IsActive())
			return;

		// We won't update constant buffer for static objects
		if (GetGameObject()->GetType() == GameObject::Type::Static)
			return;

		auto& context = D_GRAPHICS::GraphicsContext::Begin();

		// Updating mesh constants
		// Mapping upload buffer
		auto& currentUploadBuff = mMeshConstantsCPU[D_RENDERER_DEVICE::GetCurrentResourceIndex()];
		MeshConstants* cb = (MeshConstants*)currentUploadBuff.Map();

		auto world = GetTransform().GetWorld();
		cb->mWorld = Matrix4(world);
		cb->mWorldIT = InverseTranspose(Matrix3(world));

		currentUploadBuff.Unmap();

		// Uploading
		context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
		context.GetCommandList()->CopyBufferRegion(mMeshConstantsGPU.GetResource(), 0, currentUploadBuff.GetResource(), 0, currentUploadBuff.GetBufferSize());
		context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);

		context.Finish();

	}

	void SkeletalMeshRendererComponent::OnDestroy()
	{
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOUCE::gNumFrameResources; i++)
		{
			mMeshConstantsCPU[i].Destroy();
		}
		mMeshConstantsGPU.Destroy();
	}
}
