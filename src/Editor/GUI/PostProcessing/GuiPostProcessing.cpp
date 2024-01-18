#include "Editor/pch.hpp"
#include "GuiPostProcessing.hpp"

#include <Graphics/GraphicsUtils/Buffers/ColorBuffer.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>
#include <Renderer/RendererCommon.hpp>

#include "GuiPostProcessing.sgenerated.hpp"

namespace Darius::Editor::Gui::PostProcessing
{
	bool GuiPostProcessing::sInitialized = false;
	D_GRAPHICS_UTILS::ComputePSO GuiPostProcessing::sOutlineCS(L"Editor Post Effect: Selection Outline");
	D_GRAPHICS_UTILS::RootSignature GuiPostProcessing::sRootSignature;

	void GuiPostProcessing::InitializePSOs()
	{
		if (sInitialized)
			return;

		// Initialize root signature
		sRootSignature.Reset(4, 2);
		sRootSignature.InitStaticSampler(0, D_GRAPHICS::SamplerLinearClampDesc);
		sRootSignature.InitStaticSampler(1, D_GRAPHICS::SamplerLinearBorderDesc);
		sRootSignature[0].InitAsConstants(0, 10);
		sRootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 4);
		sRootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 4);
		sRootSignature[3].InitAsConstantBuffer(1);
		sRootSignature.Finalize(L"Editor Post Effect");

		// Initializing Shaders
#define CreatePSO(ObjName, ShaderName) \
	{ \
		ObjName.SetRootSignature(sRootSignature); \
		auto shaderData = D_GRAPHICS::GetShaderByName(#ShaderName); \
		ObjName.SetComputeShader(shaderData->GetBufferPointer(), shaderData->GetBufferSize()); \
		ObjName.Finalize(); \
	}

		CreatePSO(sOutlineCS, EditorSelectionOutlineCS);

#undef CreatePSO
		sInitialized = true;
	}

	GuiPostProcessing::GuiPostProcessing(UINT8 outlineStencilRef) :
		mOutlineStencilReference(outlineStencilRef),
		mRenderTargetFilter(RenderTargetFilter::Color),
		mOutlineColor(255.f, 0, 255.f),
		mCoveredOutlineMultiplier(0.2f),
		mOutlineWidth(1.f)
	{
		if (!sInitialized)
			InitializePSOs();
	}

	void GuiPostProcessing::Render(D_RENDERER::SceneRenderContext const& renderContext, D_GRAPHICS_BUFFERS::ColorBuffer& destinationBuffer)
	{
		D_ASSERT(sInitialized);

		D_PROFILING::ScopedTimer _prof(L"Gui Post Processing", renderContext.CommandContext);

		bool success = false;

		if (mRenderTargetFilter == RenderTargetFilter::Color)
		{
			success |= ApplyEditorSelectionOutline(renderContext, destinationBuffer);
		}

		// Fallback operation -> Copy src scene color to destination
		if (!success)
		{
			RECT region = { 0l, 0l, (long)destinationBuffer.GetWidth(), (long)destinationBuffer.GetHeight() };
			renderContext.CommandContext.CopyTextureRegion(destinationBuffer, 0u, 0u, 0u, renderContext.ColorBuffer, region);
		}
	}

	bool GuiPostProcessing::ApplyEditorSelectionOutline(Darius::Renderer::SceneRenderContext const& renderContext, Darius::Graphics::Utils::Buffers::ColorBuffer& destinationBuffer)
	{
		if (!renderContext.DepthBuffer.HasStencil())
			return false;

		ALIGN_DECL_16 struct
		{
			float			TexelSize[2];
			float			OutlineWidth;
			UINT			StencilRefValue;
			float			OutlineColor[3];
			float			Threshold;
		} constants =
		{
			{ 1.f / destinationBuffer.GetWidth(), 1.f / destinationBuffer.GetHeight() },
			mOutlineWidth,
			mOutlineStencilReference,
			{ mOutlineColor.GetR(), mOutlineColor.GetG(), mOutlineColor.GetB() },
			80000.f,
		};

		auto& context = renderContext.CommandContext.GetComputeContext();
		context.SetRootSignature(sRootSignature);


		context.TransitionResource(renderContext.DepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context.TransitionResource(renderContext.ColorBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context.TransitionResource(destinationBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

		context.SetPipelineState(sOutlineCS);
		context.SetDynamicDescriptor(2, 0, renderContext.DepthBuffer.GetStencilSRV());
		context.SetDynamicDescriptor(2, 1, renderContext.ColorBuffer.GetSRV());
		context.SetDynamicDescriptor(1, 0, destinationBuffer.GetUAV());
		context.SetDynamicConstantBufferView(3, sizeof(constants), &constants);
		context.Dispatch2D(destinationBuffer.GetWidth(), destinationBuffer.GetHeight());

		return true;
	}

}
