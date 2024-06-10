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
using namespace D_RENDERER;
using namespace D_RENDERER_GEOMETRY;
using namespace D_RESOURCE;
using namespace D_SERIALIZATION;

namespace Darius::Renderer
{
	D_H_COMP_DEF(MeshRendererComponent);


	MeshRendererComponent::MeshRendererComponent() :
		MeshRendererComponentBase(),
		mMesh()
	{
	}

	MeshRendererComponent::MeshRendererComponent(D_CORE::Uuid const& uuid) :
		MeshRendererComponentBase(uuid),
		mMesh()
	{
	}

	bool MeshRendererComponent::AddRenderItems(std::function<void(D_RENDERER::RenderItem const&)> appendFunction, RenderItemContext const& riContext)
	{

		if (mMesh->IsDirtyGPU())
			return false;

		bool any = false;
		auto result = RenderItem();
		const Mesh* mesh = mMesh.Get()->GetMeshData();
		result.Mesh = mesh;
		result.MeshVsCBV = GetConstantsAddress();

#if _D_EDITOR
		if (riContext.IsEditor)
		{
			if (GetGameObject() == riContext.SelectedGameObject)
			{
				result.StencilEnable = true;
				result.CustomDepth = true;
				result.StencilValue = riContext.StencilOverride;
			}
			else
			{
				result.StencilEnable = false;
				result.CustomDepth = false;
				result.StencilValue = 0;
			}
		}
		else
#endif
		{
			result.StencilEnable = IsStencilWriteEnable();
			result.StencilValue = GetStencilValue();
			result.CustomDepth = IsCustomDepthEnable();
		}

		static auto incSize = D_GRAPHICS_DEVICE::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D_CONTAINERS::DVector<ResourceRef<MaterialResource>> meshMaterials = mMesh->GetMaterials();
		UINT draws = (UINT)mesh->mDraw.size();
		if (draws != (UINT)mMaterials.size())
			OnMeshChanged();

		for (UINT i = 0; i < draws; i++)
		{
			auto const& draw = mesh->mDraw[i];

			ResourceRef<MaterialResource> material = mMaterials[i];

			if (!mMaterials[i].IsValid())
				material = meshMaterials[i];

			if (!material.IsValid() || material->IsDirtyGPU())
				continue;


			result.PsoType = GetPsoIndex(i, material.Get());
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
				result.TextureDomainSRV = { result.Material.MaterialSRV.ptr + (incSize * D_RENDERER_RAST::kWorldDisplacement) };
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

	D_MATH_BOUNDS::Aabb MeshRendererComponent::GetAabb() const
	{
		auto transform = GetTransform();

		if (!mMesh.IsValid())
			return D_MATH_BOUNDS::Aabb(transform->GetPosition());

		auto localAabb = mMesh->GetMeshData()->mBoundBox;
		auto affineTransform = AffineTransform(transform->GetWorld());

		return localAabb.CalculateTransformed(affineTransform);
	}

	UINT MeshRendererComponent::GetPsoIndex(UINT materialIndex, MaterialResource* material)
	{
		auto materialPsoFlags = material->GetPsoFlags();

		// Whether resource has changed
		if (mMaterialPsoData[materialIndex].CachedMaterialPsoFlags != materialPsoFlags)
		{
			mMaterialPsoData[materialIndex].CachedMaterialPsoFlags = materialPsoFlags;
			mMaterialPsoData[materialIndex].PsoIndexDirty = true;
		}

		// Whether pso index is not compatible with current pso flags
		if (mMaterialPsoData[materialIndex].PsoIndexDirty)
		{
			D_RENDERER_RAST::PsoConfig config;
			config.PsoFlags = materialPsoFlags | mComponentPsoFlags;
			if (material->HasDisplacement())
			{
				config.PsoFlags |= RenderItem::PointOnly | RenderItem::LineOnly; // Means patch
				config.VSIndex = D_GRAPHICS::GetShaderIndex("WorldDisplacementVS");
				config.HSIndex = D_GRAPHICS::GetShaderIndex("WorldDisplacementHS");
				config.DSIndex = D_GRAPHICS::GetShaderIndex("WorldDisplacementDS");
				config.PSIndex = D_GRAPHICS::GetShaderIndex("DefaultPS");
			}

			mMaterialPsoData[materialIndex].PsoIndex = D_RENDERER_RAST::GetPso(config);

			config.PsoFlags |= RenderItem::DepthOnly;

			if (!(config.PsoFlags & RenderItem::AlphaTest))
				config.PSIndex = 0;

			mMaterialPsoData[materialIndex].DepthPsoIndex = D_RENDERER_RAST::GetPso(config);

			mMaterialPsoData[materialIndex].PsoIndexDirty = false;
		}
		return mMaterialPsoData[materialIndex].PsoIndex;
	}

	void MeshRendererComponent::SetMesh(StaticMeshResource* mesh)
	{
		if (!CanChange())
			return;

		if (mMesh == mesh)
			return;

		mMesh = mesh;

		auto meshValid = mMesh.IsValid();

		if (meshValid && mMesh->IsLoaded())
			OnMeshChanged();
		else if (meshValid)
		{
			D_RESOURCE_LOADER::LoadResourceAsync(mesh, [&](auto resource)
				{
					OnMeshChanged();
				}, true);
		}

		mChangeSignal(this);
	}

#ifdef _D_EDITOR

	bool MeshRendererComponent::CanRenderForPicker() const
	{
		if(mMesh.IsNull() || mMesh->IsDirtyGPU())
			return false;

		return true;
	}

	RenderItem MeshRendererComponent::GetPickerRenderItem() const
	{
		static uint32_t flags = RenderItem::HasPosition | RenderItem::HasNormal | RenderItem::HasTangent | RenderItem::HasUV0;
		static RenderItem ri;
		struct PsoInitializer
		{
			PsoInitializer()
			{
				uint32_t psoIndex, depthPsoIndex;
				D_RENDERER_RAST::PsoConfig config;
				config.PsoFlags = flags;

				config.PSIndex = D_GRAPHICS::GetShaderIndex("EditorPickerPS");
				config.VSIndex = D_GRAPHICS::GetShaderIndex("DefaultVS");
				config.RenderRargetFormats = {DXGI_FORMAT_R32G32_UINT};

				psoIndex = D_RENDERER_RAST::GetPso(config);
				config.PsoFlags |= RenderItem::DepthOnly;
				config.PSIndex = 0;
				depthPsoIndex = D_RENDERER_RAST::GetPso(config);

				ri.PsoType = psoIndex;
				ri.DepthPsoIndex = depthPsoIndex;
				ri.Material.MaterialSRV = {0};
				ri.Material.SamplersSRV = {0};
				ri.PsoFlags = flags;
				ri.BaseVertexLocation = 0;
				ri.StartIndexLocation = 0;
			}
		};

		static PsoInitializer initializer;

		const Mesh* mesh = mMesh.Get()->GetMeshData();
		ri.MeshVsCBV = GetConstantsAddress();
		ri.Material.MaterialCBV = GetPickerDrawPsConstant();
		ri.Mesh = mesh;
		ri.IndexCount = mesh->mNumTotalIndices;
		
		return ri;
	}

	bool MeshRendererComponent::DrawDetails(float params[])
	{
		auto valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Mesh selection
		D_H_DETAILS_DRAW_PROPERTY("Mesh");
		D_H_RESOURCE_SELECTION_DRAW(StaticMeshResource, mMesh, "Select Mesh", SetMesh);

		D_H_DETAILS_DRAW_END_TABLE();

		valueChanged |= MeshRendererComponentBase::DrawDetails(params);

		return valueChanged;

	}
#endif

	void MeshRendererComponent::Update(float dt)
	{
		if (!IsDirty() || !IsActive())
			return;

		if (!mMesh.IsValid())
			return;

		auto& context = D_GRAPHICS::GraphicsContext::Begin();

		Super::Update(dt);

		auto frameResourceIndex = D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex();

		// Updating mesh constants
		// Mapping upload buffer
		MeshConstants* cb = (MeshConstants*)mMeshConstantsCPU.MapInstance(frameResourceIndex);

		auto world = GetTransform()->GetWorld();
		cb->World = world;
		cb->WorldIT = InverseTranspose(world.Get3x3());
		cb->Lod = mLoD;


		mMeshConstantsCPU.Unmap();

		// Uploading
		context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
		context.CopyBufferRegion(mMeshConstantsGPU, 0, mMeshConstantsCPU, mMeshConstantsCPU.GetBufferSize() * frameResourceIndex, mMeshConstantsCPU.GetBufferSize());
		context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);

		context.Finish();
		SetClean();
	}

	void MeshRendererComponent::GetOverriddenMaterials(D_CONTAINERS::DVector<MaterialResource*>& out) const
	{
		if (!mMesh.IsValid() || !mMesh->IsLoaded())
		{
			out.resize(0);
			return;
		}

		out.resize(mMaterials.size());

		for (int i = 0; i < out.size(); i++)
		{
			auto const& overrideMat = mMaterials[i];
			if (overrideMat.IsValid())
			{
				out[i] = overrideMat.Get();
				continue;
			}
			out[i] = mMesh->GetMaterial(i);
		}
	}

}
