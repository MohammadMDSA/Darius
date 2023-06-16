#include "Renderer/pch.hpp"
#include "BillboardRendererComponent.hpp"

#include "Renderer/Resources/MaterialResource.hpp"
#include "Renderer/GraphicsUtils/VertexTypes.hpp"


#ifdef _D_EDITOR
#include <Libs/FontIcon/IconsFontAwesome6.h>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif // _D_EIDITOR

#include "BillboardRendererComponent.sgenerated.hpp"

using namespace D_GRAPHICS;
using namespace D_GRAPHICS_UTILS;
using namespace D_MATH;
using namespace D_RENDERER;
using namespace D_RENDERER_FRAME_RESOURCE;

namespace
{
	ALIGN_DECL_256 struct BillboardConstants
	{
		DirectX::XMFLOAT4X4		world;
		DirectX::XMFLOAT2		size;
	};
}

namespace Darius::Graphics
{

	D_H_COMP_DEF(BillboardRendererComponent);

	BillboardRendererComponent::BillboardRendererComponent() :
		ComponentBase(),
		mBoundDirty(true),
		mWidth(1),
		mHeight(1),
		mMaterial(GetAsCountedOwner())
	{
	}

	BillboardRendererComponent::BillboardRendererComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid),
		mBoundDirty(true),
		mWidth(1),
		mHeight(1),
		mMaterial(GetAsCountedOwner())
	{
	}

	void BillboardRendererComponent::Awake()
	{
		// Initializing Mesh Constants buffers
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOURCE::gNumFrameResources; i++)
		{
			mMeshConstantsCPU[i].Create(L"Mesh Constant Upload Buffer", sizeof(BillboardConstants));
		}
		mMeshConstantsGPU.Create(L"Mesh Constant GPU Buffer", 1, sizeof(BillboardConstants));

	}

	void BillboardRendererComponent::Update(float dt)
	{
		if (!IsActive())
			return;

		if (!mMaterial.IsValid() || mMaterial->IsDirtyGPU())
			return;

		auto& context = D_GRAPHICS::GraphicsContext::Begin();

		// Updating mesh constants
		// Mapping upload buffer
		auto& currentUploadBuff = mMeshConstantsCPU[D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex()];

		BillboardConstants* cb = reinterpret_cast<BillboardConstants*>(currentUploadBuff.Map());
		auto world = GetTransform()->GetWorld();
		cb->world = world;
		cb->size = { mWidth, mHeight };
		currentUploadBuff.Unmap();

		// Uploading
		context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
		context.CopyBufferRegion(mMeshConstantsGPU, 0, currentUploadBuff, 0, currentUploadBuff.GetBufferSize());
		context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);

		context.Finish();
	}

	void BillboardRendererComponent::SetWidth(float const& value)
	{
		if (value < 0)
			return;

		this->mWidth = value;
	}

	void BillboardRendererComponent::SetHeight(float const& value)
	{
		if (value < 0)
			return;

		this->mHeight = value;
	}

	bool BillboardRendererComponent::AddRenderItems(std::function<void(D_RENDERER_FRAME_RESOURCE::RenderItem const&)> appendFunction)
	{

		if (!mMaterial.IsValid() || mMaterial->IsDirtyGPU())
			return false;

		UpdatePsoIndex();

		static const uint16_t psoFlags = mMaterial->GetPsoFlags() | RenderItem::SkipVertexIndex | RenderItem::PointOnly;

		RenderItem ri;
		ri.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		ri.MeshVsCBV = GetConstantsAddress();
		ri.PsoType = mMaterialPsoData.PsoIndex;
		ri.DepthPsoIndex = mMaterialPsoData.DepthPsoIndex;
		ri.Material.MaterialCBV = *mMaterial.Get();
		ri.Material.MaterialSRV = mMaterial->GetTexturesHandle();
		ri.Material.SamplersSRV = mMaterial->GetSamplersHandle();
		ri.PsoFlags = psoFlags;
		ri.BaseVertexLocation = 0;
		ri.IndexCount = 1;

		appendFunction(ri);

		return true;
	}

	void BillboardRendererComponent::UpdatePsoIndex()
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
			config.PsoFlags = mMaterial->GetPsoFlags() | RenderItem::SkipVertexIndex | RenderItem::PointOnly;

			config.PSIndex = GetShaderIndex("BillboardPS");
			config.VSIndex = GetShaderIndex("BillboardVS");
			config.GSIndex = GetShaderIndex("BillboardGS");

			mMaterialPsoData.PsoIndex = D_RENDERER::GetPso(config);
			config.PsoFlags |= RenderItem::DepthOnly;
			config.PSIndex = 0;
			mMaterialPsoData.DepthPsoIndex = D_RENDERER::GetPso(config);
			mMaterialPsoData.PsoIndexDirty = false;

		}

	}

#ifdef _D_EDITOR
	bool BillboardRendererComponent::DrawDetails(float[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE("Billbouard Details");

		D_H_DETAILS_DRAW_PROPERTY("Material");
		D_H_RESOURCE_SELECTION_DRAW(MaterialResource, mMaterial, "Select Material", SetMaterial);

		D_H_DETAILS_DRAW_PROPERTY("Width");
		auto width = GetWidth();
		if (ImGui::DragFloat("##Width", &width, 0.1f, 0.f))
		{
			SetWidth(width);
		}

		D_H_DETAILS_DRAW_PROPERTY("Height");
		auto height = GetHeight();
		if (ImGui::DragFloat("##Height", &height, 0.1f, 0.f))
		{
			SetHeight(height);
		}

		{
			D_H_DETAILS_DRAW_PROPERTY("Casts Shadow");
			auto casts = IsCastsShadow();
			if (ImGui::Checkbox("##CastsShadow", &casts))
			{
				SetCastsShadow(casts);
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif 
}
