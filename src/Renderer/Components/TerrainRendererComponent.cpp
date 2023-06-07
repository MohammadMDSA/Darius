#include "Renderer/pch.hpp"
#include "TerrainRendererComponent.hpp"

#include "Renderer/Resources/MaterialResource.hpp"
#include "Renderer/GraphicsUtils/VertexTypes.hpp"


#ifdef _D_EDITOR
#include <Libs/FontIcon/IconsFontAwesome6.h>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif // _D_EIDITOR

#include "TerrainRendererComponent.sgenerated.hpp"

using namespace D_GRAPHICS;
using namespace D_GRAPHICS_UTILS;
using namespace D_MATH;
using namespace D_RENDERER;
using namespace D_RENDERER_FRAME_RESOURCE;

namespace Darius::Graphics
{

	D_H_COMP_DEF(TerrainRendererComponent);

	TerrainRendererComponent::TerrainRendererComponent() :
		ComponentBase(),
		mMaterial(GetAsCountedOwner()),
		mGridMesh(GetAsCountedOwner()),
		//mHeightMap(GetAsCountedOwner()),
		mGridSize(TerrainGridSize::Cells8x8),
		mCastsShadow(true)
	{
	}

	TerrainRendererComponent::TerrainRendererComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid),
		mMaterial(GetAsCountedOwner()),
		mGridMesh(GetAsCountedOwner()),
		//mHeightMap(GetAsCountedOwner()),
		mGridSize(TerrainGridSize::Cells8x8),
		mCastsShadow(true)
	{
	}

	void TerrainRendererComponent::Awake()
	{
		// Initializing Mesh Constants buffers
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOURCE::gNumFrameResources; i++)
		{
			mMeshConstantsCPU[i].Create(L"Mesh Constant Upload Buffer", sizeof(MeshConstants));
		}
		mMeshConstantsGPU.Create(L"Mesh Constant GPU Buffer", 1, sizeof(MeshConstants));

		UpdateGridMesh();

	}

	void TerrainRendererComponent::Update(float dt)
	{
		if (!IsActive())
			return;

		if (!mMaterial.IsValid() || mMaterial->IsDirtyGPU())
			return;

		auto& context = D_GRAPHICS::GraphicsContext::Begin();

		// Updating mesh constants
		// Mapping upload buffer
		auto& currentUploadBuff = mMeshConstantsCPU[D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex()];

		MeshConstants* cb = reinterpret_cast<MeshConstants*>(currentUploadBuff.Map());
		Matrix4 world = Matrix4(GetTransform().GetWorld());
		cb->World = Matrix4(world);
		cb->WorldIT = InverseTranspose(world.Get3x3());
		currentUploadBuff.Unmap();

		// Uploading
		context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
		context.CopyBufferRegion(mMeshConstantsGPU, 0, currentUploadBuff, 0, currentUploadBuff.GetBufferSize());
		context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);

		context.Finish();
	}

	bool TerrainRendererComponent::AddRenderItems(std::function<void(D_RENDERER_FRAME_RESOURCE::RenderItem const&)> appendFunction)
	{

		if (!mTerrainData.IsValid() || !mMaterial.IsValid() || mMaterial->IsDirtyGPU())
			return false;

		UpdatePsoIndex();

		static const uint16_t psoFlags = mMaterial->GetPsoFlags() | RenderItem::PointOnly | RenderItem::LineOnly;

		RenderItem ri;
		ri.Mesh = mGridMesh->GetMeshData();
		ri.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
		ri.MeshHsCBV = GetConstantsAddress();
		ri.MeshDsCBV = GetConstantsAddress();
		ri.ParamsDsCBV = mTerrainData->GetParamsConstantsAddress();
		ri.PsoType = mMaterialPsoData.PsoIndex;
		ri.DepthPsoIndex = mMaterialPsoData.DepthPsoIndex;
		ri.Material.MaterialCBV = *mMaterial.Get();
		ri.Material.MaterialSRV = mMaterial->GetTexturesHandle();
		ri.Material.SamplersSRV = mMaterial->GetSamplersHandle();
		ri.PsoFlags = psoFlags;
		ri.BaseVertexLocation = 0;
		ri.StartIndexLocation = 0;

		ri.TextureDomainSRV = mTerrainData->GetTexturesHandle();
		ri.IndexCount = ri.Mesh->mNumTotalIndices;

		appendFunction(ri);

		return true;
	}

			void TerrainRendererComponent::UpdatePsoIndex()
	{

#define ShaderData(name) GetShaderByName(name)->GetBufferPointer(), Shaders[name]->GetBufferSize()

		auto materialPsoFlags = mMaterial->GetPsoFlags();

		// Whether resource has changed
		if (mMaterialPsoData.CachedMaterialPsoFlags != materialPsoFlags)
		{
			mMaterialPsoData.CachedMaterialPsoFlags = materialPsoFlags;
			mMaterialPsoData.PsoIndexDirty = true;
		}

		// Whether pso index is not compatible with current pso flags
		if (mMaterialPsoData.PsoIndexDirty)
		{
			PsoConfig config;
			config.PsoFlags = mMaterial->GetPsoFlags() | RenderItem::PointOnly | RenderItem::LineOnly;

			config.PSIndex = GetShaderIndex("DefaultPS");
			config.VSIndex = GetShaderIndex("TerrainVS");
			config.HSIndex = GetShaderIndex("TerrainHS");
			config.DSIndex = GetShaderIndex("TerrainDS");

			mMaterialPsoData.PsoIndex = D_RENDERER::GetPso(config);
			config.PsoFlags |= RenderItem::DepthOnly;
			config.PSIndex = 0;
			mMaterialPsoData.DepthPsoIndex = D_RENDERER::GetPso(config);
			mMaterialPsoData.PsoIndexDirty = false;
		}

	}

	void TerrainRendererComponent::SetGridSize(TerrainGridSize size)
	{
		this->mGridSize = size;
		UpdateGridMesh();
	}

	void TerrainRendererComponent::UpdateGridMesh()
	{
		D_GRAPHICS::DefaultResource defaultResourceType;
		switch (mGridSize)
		{
		case Darius::Graphics::TerrainRendererComponent::TerrainGridSize::Cells2x2:
			defaultResourceType = D_GRAPHICS::DefaultResource::GridPatch2x2Mesh;
			break;
		case Darius::Graphics::TerrainRendererComponent::TerrainGridSize::Cells4x4:
			defaultResourceType = D_GRAPHICS::DefaultResource::GridPatch4x4Mesh;
			break;
		case Darius::Graphics::TerrainRendererComponent::TerrainGridSize::Cells8x8:
			defaultResourceType = D_GRAPHICS::DefaultResource::GridPatch8x8Mesh;
			break;
		case Darius::Graphics::TerrainRendererComponent::TerrainGridSize::Cells16x16:
			defaultResourceType = D_GRAPHICS::DefaultResource::GridPatch16x16Mesh;
			break;
		default:
			D_ASSERT_M(false, "Bad gird size");
			defaultResourceType = D_GRAPHICS::DefaultResource::GridPatch2x2Mesh;
		}

		mGridMesh = D_RESOURCE::GetResource<D_GRAPHICS::StaticMeshResource>(D_GRAPHICS::GetDefaultGraphicsResource(defaultResourceType), GetAsCountedOwner());
	}


#ifdef _D_EDITOR
	bool TerrainRendererComponent::DrawDetails(float[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE("Terrain Details");

		{
			D_H_DETAILS_DRAW_PROPERTY("Data");
			D_H_RESOURCE_SELECTION_DRAW(TerrainResource, mTerrainData, "Select Terrain Data", SetTerrainData);
		}

		{
			D_H_DETAILS_DRAW_PROPERTY("Material");
			D_H_RESOURCE_SELECTION_DRAW(MaterialResource, mMaterial, "Select Material", SetMaterial);
		}

		{
			D_H_DETAILS_DRAW_PROPERTY("Grid Division Size");
			if (ImGui::BeginCombo("##GridDivisionSize", TerrainRendererComponent::GetTerrainSizeName(GetGridSize()).c_str()))
			{

				if (ImGui::Selectable("2x2", mGridSize == TerrainGridSize::Cells2x2))
				{
					SetGridSize(TerrainGridSize::Cells2x2);
				}

				if (ImGui::Selectable("4x4", mGridSize == TerrainGridSize::Cells4x4))
				{
					SetGridSize(TerrainGridSize::Cells4x4);
				}

				if (ImGui::Selectable("8x8", mGridSize == TerrainGridSize::Cells8x8))
				{
					SetGridSize(TerrainGridSize::Cells8x8);
				}

				if (ImGui::Selectable("16x16", mGridSize == TerrainGridSize::Cells16x16))
				{
					SetGridSize(TerrainGridSize::Cells16x16);
				}

				ImGui::EndCombo();
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif 
	}
