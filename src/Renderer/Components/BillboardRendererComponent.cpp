#include "Renderer/pch.hpp"
#include "BillboardRendererComponent.hpp"

#include "Renderer/Resources/MaterialResource.hpp"
#include "Renderer/VertexTypes.hpp"

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

namespace
{
	ALIGN_DECL_256 struct BillboardConstants
	{
		DirectX::XMFLOAT4X4		world;
		DirectX::XMFLOAT2		size;
	};
}

namespace Darius::Renderer
{

	D_H_COMP_DEF(BillboardRendererComponent);

	BillboardRendererComponent::BillboardRendererComponent() :
		RendererComponent(),
		mWidth(1),
		mHeight(1),
		mMaterial()
	{
	}

	BillboardRendererComponent::BillboardRendererComponent(D_CORE::Uuid const& uuid) :
		RendererComponent(uuid),
		mWidth(1),
		mHeight(1),
		mMaterial()
	{
	}

	void BillboardRendererComponent::Awake()
	{
		Super::Awake();

		// Initializing Mesh Constants buffers
		mMeshConstantsCPU.Create(L"Mesh Constant Upload Buffer", sizeof(BillboardConstants), D_GRAPHICS_DEVICE::gNumFrameResources);
		mMeshConstantsGPU.Create(L"Mesh Constant GPU Buffer", 1, sizeof(BillboardConstants));

	}

	void BillboardRendererComponent::Update(float dt)
	{
		if (!IsDirty() || !IsActive())
			return;

		auto& context = D_GRAPHICS::GraphicsContext::Begin();

		Super::Update(dt);

		// Updating mesh constants
		// Mapping upload buffer
		auto frameResourceIndex = D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex();
		BillboardConstants* cb = reinterpret_cast<BillboardConstants*>(mMeshConstantsCPU.MapInstance(frameResourceIndex));
		auto& world = GetTransform()->GetWorld();
		cb->world = world;
		cb->size = { mWidth, mHeight };
		mMeshConstantsCPU.Unmap();

		// Uploading
		context.UploadToBuffer(mMeshConstantsGPU, mMeshConstantsCPU);
		context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);

		context.Finish();
		SetClean();
	}

	void BillboardRendererComponent::SetWidth(float const& value)
	{
		if (!CanChange())
			return;

		if (value < 0)
			return;

		this->mWidth = value;

		SetDirty();
	}

	void BillboardRendererComponent::SetHeight(float const& value)
	{
		if (!CanChange())
			return;

		if (value < 0)
			return;

		this->mHeight = value;

		SetDirty();
	}

	void BillboardRendererComponent::OnDestroy()
	{
		mMeshConstantsCPU.Destroy();
		mMeshConstantsGPU.Destroy();

		Super::OnDestroy();
	}

	bool BillboardRendererComponent::AddRenderItems(std::function<void(D_RENDERER::RenderItem const&)> appendFunction, RenderItemContext const& riContext)
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

#if _D_EDITOR
		if (riContext.IsEditor)
		{
			if (GetGameObject() == riContext.SelectedGameObject)
			{
				ri.StencilEnable = true;
				ri.CustomDepth = true;
				ri.StencilValue = riContext.StencilOverride;
			}
			else
			{
				ri.StencilEnable = false;
				ri.CustomDepth = false;
				ri.StencilValue = 0;
			}
		}
		else
#endif
		{
			ri.CustomDepth = IsCustomDepthEnable();
			ri.StencilEnable = IsStencilWriteEnable();
			ri.StencilValue = GetStencilValue();

		}

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
			D_RENDERER_RAST::PsoConfig config;
			config.PsoFlags = mMaterial->GetPsoFlags() | RenderItem::SkipVertexIndex | RenderItem::PointOnly;

			config.PSIndex = GetShaderIndex("BillboardPS");
			config.VSIndex = GetShaderIndex("BillboardVS");
			config.GSIndex = GetShaderIndex("BillboardGS");

			mMaterialPsoData.PsoIndex = D_RENDERER_RAST::GetPso(config);
			config.PsoFlags |= RenderItem::DepthOnly;
			config.PSIndex = 0;
			mMaterialPsoData.DepthPsoIndex = D_RENDERER_RAST::GetPso(config);
			mMaterialPsoData.PsoIndexDirty = false;

		}
	}

	void BillboardRendererComponent::SetMaterial(MaterialResource* material)
	{
		if (mMaterial == material)
			return;

		mMaterial = material;

		if (mMaterial.IsValid() && !mMaterial->IsLoaded())
			D_RESOURCE_LOADER::LoadResourceAsync(material, nullptr, true);

		mChangeSignal(this);
	}

	D_MATH_BOUNDS::Aabb BillboardRendererComponent::GetAabb() const
	{
		float radius = D_MATH::Length(D_MATH::Vector3::Up * mWidth + D_MATH::Vector3::Forward * mHeight);
		return D_MATH_BOUNDS::Aabb::CreateFromCenterAndExtents(GetTransform()->GetPosition(), Vector3(radius));
	}

#ifdef _D_EDITOR

	RenderItem BillboardRendererComponent::GetPickerRenderItem() const
	{
		static RenderItem ri;
		struct PsoInitializer
		{
			PsoInitializer()
			{
				uint32_t psoIndex, depthPsoIndex;
				uint32_t flags = RenderItem::HasPosition | RenderItem::HasNormal | RenderItem::HasTangent | RenderItem::HasUV0 | RenderItem::SkipVertexIndex | RenderItem::PointOnly;

				D_RENDERER_RAST::PsoConfig config;
				config.PsoFlags = flags;

				config.PSIndex = GetShaderIndex("EditorPickerPS");
				config.VSIndex = GetShaderIndex("BillboardVS");
				config.GSIndex = GetShaderIndex("BillboardGS");
				config.RenderRargetFormats = {DXGI_FORMAT_R32G32_UINT};

				psoIndex = D_RENDERER_RAST::GetPso(config);
				config.PsoFlags |= RenderItem::DepthOnly;
				config.PSIndex = 0;
				depthPsoIndex = D_RENDERER_RAST::GetPso(config);

				ri.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
				ri.PsoType = psoIndex;
				ri.DepthPsoIndex = depthPsoIndex;
				ri.Material.MaterialSRV = {0};
				ri.Material.SamplersSRV = {0};
				ri.PsoFlags = flags;
				ri.BaseVertexLocation = 0;
				ri.IndexCount = 1;
			}
		};

		static PsoInitializer initializer;
		ri.MeshVsCBV = GetConstantsAddress();
		ri.Material.MaterialCBV = GetPickerDrawPsConstant();

		return ri;
	}

	bool BillboardRendererComponent::DrawDetails(float params[])
	{
		bool valueChanged = false;

		valueChanged |= Super::DrawDetails(params);

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

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

}
