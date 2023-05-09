#include "Renderer/pch.hpp"
#include "BillboardRendererComponent.hpp"

#include "Renderer/Resources/MaterialResource.hpp"

#include <Utils/DragDropPayload.hpp>

#ifdef _D_EDITOR
#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>
#endif // _D_EIDITOR

#include "BillboardRendererComponent.sgenerated.hpp"

using namespace D_GRAPHICS;
using namespace D_GRAPHICS_UTILS;
using namespace D_MATH;
using namespace D_RENDERER_FRAME_RESOURCE;

namespace Darius::Graphics
{
	UINT BillboardRendererComponent::RenderPsoIndex = 0u;
	UINT BillboardRendererComponent::DepthPsoIndex = 0u;

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
			mMeshConstantsCPU[i].Create(L"Mesh Constant Upload Buffer", sizeof(MeshConstants));
		}
		mMeshConstantsGPU.Create(L"Mesh Constant GPU Buffer", 1, sizeof(MeshConstants));

	}

	void BillboardRendererComponent::Update(float dt)
	{
		if (!IsActive())
			return;

		if (!mMaterial.IsValid())
			return;

		auto& context = D_GRAPHICS::GraphicsContext::Begin();

		// Updating mesh constants
		// Mapping upload buffer
		auto& currentUploadBuff = mMeshConstantsCPU[D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex()];

		ALIGN_DECL_256 struct BillboardConstants
		{
			DirectX::XMFLOAT4X4		world;
			DirectX::XMFLOAT2		size;
		};


		BillboardConstants* cb = reinterpret_cast<BillboardConstants*>(currentUploadBuff.Map());
		auto world = GetTransform().GetWorld();
		cb->world = Matrix4(world);
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

		if (!mMaterial.IsValid())
			return false;

		if (RenderPsoIndex == 0 || DepthPsoIndex == 0)
			UpdatePso();

		static const uint16_t psoFlags = RenderItem::AlphaBlend | RenderItem::SkipVertexIndex;

		RenderItem ri;
		ri.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		ri.MeshCBV = GetConstantsAddress();
		ri.PsoType = BillboardRendererComponent::RenderPsoIndex;
		ri.DepthPsoIndex = BillboardRendererComponent::DepthPsoIndex;
		ri.Material.MaterialCBV = *mMaterial.Get();
		ri.Material.MaterialSRV = mMaterial->GetTexturesHandle();
		ri.Material.SamplersSRV = mMaterial->GetSamplersHandle();
		ri.PsoFlags = psoFlags;
		ri.BaseVertexLocation = 0;
		ri.IndexCount = 1;

		appendFunction(ri);

		return true;
	}

	void BillboardRendererComponent::UpdatePso()
	{

#define ShaderData(name) Shaders[name]->GetBufferPointer(), Shaders[name]->GetBufferSize()

		GraphicsPSO renderPso;
		renderPso.SetInputLayout(0, nullptr);
		renderPso.SetVertexShader(ShaderData("BillboardVS"));
		renderPso.SetGeometryShader(ShaderData("BillboardGS"));
		renderPso.SetPixelShader(ShaderData("BillboardPS"));
		renderPso.SetRasterizerState(D_GRAPHICS::RasterizerDefault);
		renderPso.SetBlendState(D_GRAPHICS::BlendTraditional);
		renderPso.SetDepthStencilState(D_GRAPHICS::DepthStateReadOnly);
		renderPso.SetSampleMask(UINT_MAX);
		renderPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);

		GraphicsPSO depthPso = renderPso;
		depthPso.SetBlendState(BlendDisable);
		depthPso.SetPixelShader(nullptr, 0);

		D_RENDERER::AllocatePso(L"Billboard PSO", renderPso, depthPso, BillboardRendererComponent::RenderPsoIndex, BillboardRendererComponent::DepthPsoIndex);

#undef ShaderData
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

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif 
}
