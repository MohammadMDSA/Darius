#include "Scene/pch.hpp"
#include "MeshRendererComponent.hpp"
#include "Scene/Utils/DetailsDrawer.hpp"

#include <ResourceManager/ResourceManager.hpp>

#include <imgui.h>

using namespace D_SERIALIZATION;
using namespace D_RESOURCE;
using namespace D_RENDERER_FRAME_RESOUCE;

namespace Darius::Scene::ECS::Components
{
	D_H_COMP_DEF(MeshRendererComponent);


	MeshRendererComponent::MeshRendererComponent() :
		ComponentBase()
	{
	}

	MeshRendererComponent::MeshRendererComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid)
	{
	}

	void MeshRendererComponent::Start()
	{

		_SetMesh({ ResourceType::None, 0 });
		_SetMaterial(D_RESOURCE::GetDefaultResource(DefaultResource::DefaultMaterial));

	}

	RenderItem MeshRendererComponent::GetRenderItem()
	{
		auto result = RenderItem();
		const Mesh* mesh = mMeshResource.Get()->GetData();
		result.BaseVertexLocation = mesh->mDraw[0].BaseVertexLocation;
		result.IndexCount = mesh->mDraw[0].IndexCount;
		result.StartIndexLocation = mesh->mDraw[0].StartIndexLocation;
		result.Mesh = mesh;
		result.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		result.MeshCBV = GetGameObject()->GetConstantsAddress();
		result.MaterialCBV = *mMaterialResource.Get();
		return result;
	}

#ifdef _D_EDITOR
	bool MeshRendererComponent::DrawDetails(float params[])
	{
		auto changeValue = false;

		{
			MeshResource* currentMesh = mMeshResource.Get();

			if (ImGui::Button("Select"))
			{
				ImGui::OpenPopup("Select Res");
			}

			if (ImGui::BeginPopup("Select Res"))
			{
				auto meshes = D_RESOURCE::GetResourcePreviews(D_RESOURCE::ResourceType::Mesh);
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

		D_SCENE_DET_DRAW::DrawDetails(*mMaterialResource->ModifyData(), nullptr);

		{
			MaterialResource* currentMaterial = mMaterialResource.Get();

			if (ImGui::Button("Select M"))
			{
				ImGui::OpenPopup("Select Mat");
			}

			if (ImGui::BeginPopup("Select Mat"))
			{
				auto meshes = D_RESOURCE::GetResourcePreviews(D_RESOURCE::ResourceType::Material);
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
		}

		return changeValue;

	}
#endif


	void MeshRendererComponent::SetMesh(ResourceHandle handle)
	{
		mChangeSignal();
		_SetMesh(handle);
	}

	void MeshRendererComponent::SetMaterial(ResourceHandle handle)
	{
		mChangeSignal();
		_SetMaterial(handle);
	}

	void MeshRendererComponent::_SetMesh(ResourceHandle handle)
	{
		mMeshResource = D_RESOURCE::GetResource<MeshResource>(handle, *GetGameObject());
	}

	void MeshRendererComponent::_SetMaterial(ResourceHandle handle)
	{
		mMaterialResource = D_RESOURCE::GetResource<MaterialResource>(handle, *GetGameObject());
	}

	void MeshRendererComponent::Serialize(Json& j) const
	{
		D_CORE::to_json(j["Material"], mMaterialResource.Get()->GetUuid());
		D_CORE::to_json(j["Mesh"], mMeshResource.Get()->GetUuid());
	}

	void MeshRendererComponent::Deserialize(Json const& j)
	{
		auto go = GetGameObject();

		// Loading material
		Uuid materialUuid;
		D_CORE::from_json(j["Material"], materialUuid);
		mMaterialResource = D_RESOURCE::GetResource<MaterialResource>(materialUuid, *go);

		// Loading mesh
		Uuid meshUuid;
		D_CORE::from_json(j["Mesh"], meshUuid);
		mMeshResource = D_RESOURCE::GetResource<MeshResource>(meshUuid, *go);
	}

}
