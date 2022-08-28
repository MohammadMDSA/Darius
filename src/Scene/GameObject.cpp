#include "pch.hpp"
#include <Renderer/pch.hpp>
#include "GameObject.hpp"
#include "Scene/Utils/DetailsDrawer.hpp"
#include "Scene.hpp"
#include "Serialization/Serializer.hpp"
#include "EntityComponentSystem/Components/TransformComponent.hpp"

#include <Core/Uuid.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Renderer/RenderDeviceManager.hpp>

#include <imgui.h>

using namespace D_RESOURCE;
using namespace D_ECS_COMP;

namespace Darius::Scene
{

	GameObject::GameObject(Uuid uuid, D_ECS::Entity entity) :
		mActive(true),
		mType(Type::Movable),
		mName("GameObject"),
		mUuid(uuid),
		mEntity(entity)
	{
		SetMesh({ ResourceType::None, 0 });
		SetMaterial(D_RESOURCE::GetDefaultResource(DefaultResource::DefaultMaterial));

		// Initializing Mesh Constants buffers
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOUCE::gNumFrameResources; i++)
		{
			mMeshConstantsCPU[i].Create(L"Mesh Constant Upload Buffer", sizeof(MeshConstants));
		}
		mMeshConstantsGPU.Create(L"Mesh Constant GPU Buffer", 1, sizeof(MeshConstants));
		mEntity.add(D_WORLD::GetRegistry().component(D_ECS_COMP::TransformComponent::GetName().c_str()));
	}

	GameObject::~GameObject()
	{
	}

	RenderItem GameObject::GetRenderItem()
	{
		auto result = RenderItem();
		const Mesh* mesh = mMeshResource.Get()->GetData();
		result.BaseVertexLocation = mesh->mDraw[0].BaseVertexLocation;
		result.IndexCount = mesh->mDraw[0].IndexCount;
		result.StartIndexLocation = mesh->mDraw[0].StartIndexLocation;
		result.Mesh = mesh;
		result.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		result.MeshCBV = mMeshConstantsGPU.GetGpuVirtualAddress();
		result.MaterialCBV = *mMaterialResouce.Get();
		return result;
	}

	void GameObject::Update(D_GRAPHICS::GraphicsContext& context, float deltaTime)
	{
		// We won't update constant buffer for static objects
		if (mType == Type::Static)
			return;

		// Updating mesh constants
		// Mapping upload buffer
		auto& currentUploadBuff = mMeshConstantsCPU[D_RENDERER_DEVICE::GetCurrentResourceIndex()];
		MeshConstants* cb = (MeshConstants*)currentUploadBuff.Map();

		cb->mWorld = Matrix4(GetTransform()->GetWorld());

		currentUploadBuff.Unmap();

		// Uploading
		context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
		context.GetCommandList()->CopyBufferRegion(mMeshConstantsGPU.GetResource(), 0, mMeshConstantsCPU->GetResource(), 0, mMeshConstantsCPU->GetBufferSize());
		context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);

	}

#ifdef _D_EDITOR
	bool GameObject::DrawDetails(float params[])
	{
		bool changeValue = false;

		auto& reg = D_WORLD::GetRegistry();
		

		mEntity.each([&](flecs::id compId)
			{
				if (!reg.is_valid(compId))
					return;
				auto compP = mEntity.get_mut(compId);
				try
				{
					auto comp = reinterpret_cast<ComponentBase*>(compP);
					D_SCENE_DET_DRAW::DrawDetails(*comp, nullptr);

				}
				catch (const std::exception&)
				{
					D_LOG_ERROR("Error drawing component");
				}
			});

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

		D_SCENE_DET_DRAW::DrawDetails(*mMaterialResouce->ModifyData(), nullptr);

		{
			MaterialResource* currentMaterial = mMaterialResouce.Get();

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
#endif // _EDITOR

	void GameObject::SetMesh(ResourceHandle handle)
	{
		mMeshResource = D_RESOURCE::GetResource<MeshResource>(handle, *this);

	}

	void GameObject::SetMaterial(ResourceHandle handle)
	{
		mMaterialResouce = D_RESOURCE::GetResource<MaterialResource>(handle, *this);

	}

	void GameObject::SetTransform(Transform const& trans)
	{
		GetComponent<Darius::Scene::ECS::Components::TransformComponent>()->SetTransform(trans);
	}

	Transform const* GameObject::GetTransform() const
	{
		return mEntity.get<Darius::Scene::ECS::Components::TransformComponent>()->GetData();
	}

	void to_json(D_SERIALIZATION::Json& j, const GameObject& value) {
		D_H_SERIALIZE(Active);
		D_H_SERIALIZE(Name);
		D_H_SERIALIZE(Type);
		D_H_SERIALIZE(Uuid);
		j["Material"] = value.mMaterialResouce.Get()->GetUuid();
		j["Mesh"] = value.mMeshResource.Get()->GetUuid();
	}

	void from_json(const D_SERIALIZATION::Json& j, GameObject& value) {
		D_H_DESERIALIZE(Active);
		D_H_DESERIALIZE(Name);
		D_H_DESERIALIZE(Type);

		// Loading material
		Uuid materialUuid;
		D_CORE::from_json(j["Material"], materialUuid);
		value.mMaterialResouce = D_RESOURCE::GetResource<MaterialResource>(materialUuid, value);

		// Loading mesh
		Uuid meshUuid;
		D_CORE::from_json(j["Mesh"], meshUuid);
		value.mMeshResource = D_RESOURCE::GetResource<MeshResource>(meshUuid, value);
	}

}
