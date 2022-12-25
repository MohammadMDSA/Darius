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
		ComponentBase(),
		mComponentPsoFlags(0),
		mCachedMaterialPsoFlags(0),
		mPsoIndex(0),
		mPsoIndexDirty(true)
	{
	}

	MeshRendererComponent::MeshRendererComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid),
		mComponentPsoFlags(0),
		mCachedMaterialPsoFlags(0),
		mPsoIndex(0),
		mPsoIndexDirty(true)
	{
	}

	void MeshRendererComponent::Start()
	{

		// Initializing Mesh Constants buffers
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOUCE::gNumFrameResources; i++)
		{
			mMeshConstantsCPU[i].Create(L"Mesh Constant Upload Buffer", sizeof(MeshConstants));
		}
		mMeshConstantsGPU.Create(L"Mesh Constant GPU Buffer", 1, sizeof(MeshConstants));

		if (!mMaterialResource.IsValid())
			_SetMaterial(D_GRAPHICS::GetDefaultGraphicsResource(DefaultResource::Material));
	}

	RenderItem MeshRendererComponent::GetRenderItem()
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
		result.PsoType = GetPsoIndex();
		result.PsoFlags = mComponentPsoFlags | mMaterialResource->GetPsoFlags();
		return result;
	}

#ifdef _D_EDITOR
	bool MeshRendererComponent::DrawDetails(float params[])
	{
		auto valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Mesh selection
		D_H_DETAILS_DRAW_PROPERTY("Mesh");
		D_H_RESOURCE_SELECTION_DRAW(StaticMeshResource, mMeshResource, "Select Mesh", SetMesh);


		// Material selection
		D_H_DETAILS_DRAW_PROPERTY("Material");
		D_H_RESOURCE_SELECTION_DRAW(MaterialResource, mMaterialResource, "Select Material", SetMaterial);

		// Casting shadow
		D_H_DETAILS_DRAW_PROPERTY("Casts Shadow");
		valueChanged |= ImGui::Checkbox("##CastsShadow", &mCastsShadow);

		D_H_DETAILS_DRAW_END_TABLE();

		if (valueChanged)
			mChangeSignal();
		return valueChanged;

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
		mMeshResource = D_RESOURCE::GetResource<StaticMeshResource>(handle, *this);
	}

	void MeshRendererComponent::_SetMaterial(ResourceHandle handle)
	{
		mMaterialResource = D_RESOURCE::GetResource<MaterialResource>(handle, *this);
	}

	void MeshRendererComponent::Serialize(Json& j) const
	{
		if (mMaterialResource.IsValid())
			D_CORE::to_json(j["Material"], mMaterialResource.Get()->GetUuid());
		if (mMeshResource.IsValid())
			D_CORE::to_json(j["Mesh"], mMeshResource.Get()->GetUuid());
	}

	void MeshRendererComponent::Deserialize(Json const& j)
	{
		auto go = GetGameObject();

		// Loading material
		if (j.contains("Material"))
		{
			Uuid materialUuid;
			D_CORE::from_json(j["Material"], materialUuid);
			_SetMaterial(*D_RESOURCE::GetResource<MaterialResource>(materialUuid, *go));
		}

		if (j.contains("Mesh"))
		{
			// Loading mesh
			Uuid meshUuid;
			D_CORE::from_json(j["Mesh"], meshUuid);
			_SetMesh(*D_RESOURCE::GetResource<StaticMeshResource>(meshUuid, *go));
		}
	}

	void MeshRendererComponent::Update(float dt)
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

	void MeshRendererComponent::OnDestroy()
	{
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOUCE::gNumFrameResources; i++)
		{
			mMeshConstantsCPU[i].Destroy();
		}
		mMeshConstantsGPU.Destroy();
	}
}
