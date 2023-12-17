#pragma once

#include <Graphics/GraphicsUtils/PipelineState.hpp>
#include <Graphics/GraphicsUtils/RootSignature.hpp>
#include <Math/Color.hpp>
#include <Utils/Common.hpp>

#include "GuiPostProcessing.generated.hpp"

namespace Darius::Renderer
{
	struct SceneRenderContext;
}

namespace Darius::Graphics::Utils::Buffers
{
	class ColorBuffer;
}

namespace Darius::Editor::Gui::PostProcessing
{
	enum class RenderTargetFilter
	{
		Color,
		Depth,
		Normal,
		Stencil,
		CustomDepth
	};

	class DClass() GuiPostProcessing
	{
	public:
		GuiPostProcessing(UINT8 outlineStencilRef);

		void									Render(Darius::Renderer::SceneRenderContext const& renderContext, Darius::Graphics::Utils::Buffers::ColorBuffer& destinationBuffer);

	private:
		void									ApplyEditorSelectionOutline(Darius::Renderer::SceneRenderContext const& renderContext, Darius::Graphics::Utils::Buffers::ColorBuffer & destinationBuffer);

		static void								InitializePSOs();

		const UINT8								mOutlineStencilReference;
		D_MATH::Color							mOutlineColor;
		float									mOutlineWidth;
		float									mCoveredOutlineMultiplier;
		RenderTargetFilter						mRenderTargetFilter;

		static bool								sInitialized;
		static D_GRAPHICS_UTILS::ComputePSO		sOutlineCS;
		static D_GRAPHICS_UTILS::RootSignature	sRootSignature;
	};
}
