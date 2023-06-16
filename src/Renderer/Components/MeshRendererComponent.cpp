#include "Renderer/pch.hpp"
#include "MeshRendererComponent.hpp"

#include <ResourceManager/ResourceManager.hpp>
#include <Scene/Utils/DetailsDrawer.hpp>
#include <Utils/DragDropPayload.hpp>

#ifdef _D_EDITOR
#include <Libs/FontIcon/IconsFontAwesome6.h>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "MeshRendererComponent.sgenerated.hpp"

using namespace D_CORE;
using namespace D_MATH;
using namespace D_SERIALIZATION;
using namespace D_RESOURCE;
using namespace D_RENDERER_FRAME_RESOURCE;
using namespace D_RENDERER_GEOMETRY;

namespace Darius::Graphics
{
	D_H_COMP_DEF(MeshRendererComponent);


	MeshRendererComponent::MeshRendererComponent() :
		MeshRendererComponentBase(),
		mMesh(GetAsCountedOwner())
	{
	}

	MeshRendererComponent::MeshRendererComponent(D_CORE::Uuid uuid) :
		MeshRendererComponentBase(uuid),
		mMesh(GetAsCountedOwner())
	{
	}

	bool MeshRendererComponent::AddRenderItems(std::function<void(D_RENDERER_FRAME_RESOURCE::RenderItem const&)> appendFunction)
	{
		bool any = false;
		auto result = RenderItem();
		const Mesh* mesh = mMesh.Get()->GetMeshData();
		result.Mesh = mesh;
		result.MeshVsCBV = GetConstantsAddress();

		static auto incSize = D_GRAPHICS_DEVICE::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		for (UINT i = 0; i < mesh->mDraw.size(); i++)
		{
			auto const& draw = mesh->mDraw[i];
			auto const& material = mMaterials[i];

			if (!material.IsValid() || material->IsDirtyGPU())
				continue;

			result.PsoType = GetPsoIndex(i);
			result.DepthPsoIndex = mMaterialPsoData[i].DepthPsoIndex;
			result.Material.MaterialCBV = *material.Get();
			result.Material.MaterialSRV = material->GetTexturesHandle();
			result.Material.SamplersSRV = material->GetSamplersHandle();

			if (material->HasDisplacement())
			{
				result.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
				result.ParamsDsCBV = *material.Get();
				result.MeshHsCBV = GetConstantsAddress();
				result.MeshDsCBV = GetConstantsAddress();
				result.TextureDomainSRV = { result.Material.MaterialSRV.ptr + (incSize * D_RENDERER::kWorldDisplacement) };
			}
			else
				result.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

			result.PsoFlags = mComponentPsoFlags | material->GetPsoFlags();
			result.BaseVertexLocation = mesh->mDraw[i].BaseVertexLocation;
			result.StartIndexLocation = mesh->mDraw[i].StartIndexLocation;
			result.IndexCount = mesh->mDraw[i].IndexCount;

			appendFunction(result);
			any = true;
		}

		return any;
	}

	UINT MeshRendererComponent::GetPsoIndex(UINT materialIndex)
	{
		auto materialPsoFlags = mMaterials[materialIndex]->GetPsoFlags();

		// Whether resource has changed
		if (mMaterialPsoData[materialIndex].CachedMaterialPsoFlags != materialPsoFlags)
		{
			mMaterialPsoData[materialIndex].CachedMaterialPsoFlags = materialPsoFlags;
			mMaterialPsoData[materialIndex].PsoIndexDirty = true;
		}

		// Whether pso index is not compatible with current pso flags
		if (mMaterialPsoData[materialIndex].PsoIndexDirty)
		{
			D_RENDERER::PsoConfig config;
			config.PsoFlags = materialPsoFlags | mComponentPsoFlags;
			if (mMaterials[materialIndex]->HasDisplacement())
			{
				config.PsoFlags |= RenderItem::PointOnly | RenderItem::LineOnly; // Means patch
				config.VSIndex = D_GRAPHICS::GetShaderIndex("WorldDisplacementVS");
				config.HSIndex = D_GRAPHICS::GetShaderIndex("WorldDisplacementHS");
				config.DSIndex = D_GRAPHICS::GetShaderIndex("WorldDisplacementDS");
				config.PSIndex = D_GRAPHICS::GetShaderIndex("DefaultPS");
			}

			mMaterialPsoData[materialIndex].PsoIndex = D_RENDERER::GetPso(config);

			config.PsoFlags |= RenderItem::DepthOnly;
			mMaterialPsoData[materialIndex].DepthPsoIndex = D_RENDERER::GetPso(config);

			mMaterialPsoData[materialIndex].PsoIndexDirty = false;
		}
		return mMaterialPsoData[materialIndex].PsoIndex;
	}

	void MeshRendererComponent::_SetMesh(D_RESOURCE::ResourceHandle handle)
	{
		mMesh = D_RESOURCE::GetResource<StaticMeshResource>(handle, *this);
		OnMeshChanged();
	}

#ifdef _D_EDITOR
	bool MeshRendererComponent::DrawDetails(float params[])
	{
		auto valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		valueChanged |= MeshRendererComponentBase::DrawDetails(params);

		// Mesh selection
		D_H_DETAILS_DRAW_PROPERTY("Mesh");
		D_H_RESOURCE_SELECTION_DRAW(StaticMeshResource, mMesh, "Select Mesh", SetMesh);

		D_H_DETAILS_DRAW_END_TABLE();

		if (valueChanged)
			mChangeSignal();
		return valueChanged;

	}
#endif

	void MeshRendererComponent::Update(float dt)
	{
		if (!IsActive())
			return;

		if (!mMesh.IsValid())
			return;

		auto& context = D_GRAPHICS::GraphicsContext::Begin();

		// Updating mesh constants
		// Mapping upload buffer
		auto& currentUploadBuff = mMeshConstantsCPU[D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex()];
		MeshConstants* cb = (MeshConstants*)currentUploadBuff.Map();

		auto world = GetTransform()->GetWorld();
		cb->World = world;
		cb->WorldIT = InverseTranspose(world.Get3x3());
		cb->Lod = mLoD;


		currentUploadBuff.Unmap();

		// Uploading
		context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
		context.CopyBufferRegion(mMeshConstantsGPU, 0, currentUploadBuff, 0, currentUploadBuff.GetBufferSize());
		context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);

		context.Finish();

	}

}
