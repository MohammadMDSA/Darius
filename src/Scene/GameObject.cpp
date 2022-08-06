#include "pch.hpp"
#include <Renderer/pch.hpp>
#include "GameObject.hpp"

#include <ResourceManager/ResourceManager.hpp>

#include <imgui.h>

using namespace D_RESOURCE;

namespace Darius::Scene
{

	GameObject::GameObject() :
		mActive(false),
		mName("GameObject")
	{
		mTransform = Transform::MakeIdentity();
	}

	GameObject::~GameObject()
	{
	}

	RenderItem GameObject::GetRenderItem()
	{
		auto result = RenderItem();
		Mesh* mesh = *mMeshResource.Get();
		result.BaseVertexLocation = mesh->mDraw[0].BaseVertexLocation;
		result.IndexCount = mesh->mDraw[0].IndexCount;
		result.StartIndexLocation = mesh->mDraw[0].StartIndexLocation;
		result.Mesh = mesh;
		result.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		result.World = mTransform;
		return result;
	}

#ifdef _D_EDITOR
	void GameObject::DrawInspector()
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

				if (ImGui::Selectable((STR_WSTR(prev.Name) + std::to_string(idx)).c_str(), &selected))
					mMeshResource = D_RESOURCE::GetResource<MeshResource>(prev.Handle, *this);
				idx++;
			}

			ImGui::EndPopup();
		}

	}
#endif // _EDITOR

}
