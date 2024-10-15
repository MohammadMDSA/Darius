#pragma once

#include "Renderer/RendererCommon.hpp"
#include "RayTracingCommandContext.hpp"

#include <Core/Serialization/Json.hpp>
#include <Graphics/CommandContext.hpp>
#include <Graphics/GraphicsUtils/Memory/DescriptorHeap.hpp>
#include <Math/Camera/Camera.hpp>

#define D_RENDERER_RT Darius::Renderer::RayTracing

namespace Darius::Renderer::RayTracing
{

	/*struct SceneRenderContext
	{
		D_GRAPHICS_BUFFERS::ColorBuffer&					ColorBuffer;
		RayTracingCommandContext&							RayTracingContext;
		D_MATH_CAMERA::Camera const&						Camera;
		bool												DrawSkybox;
	};*/

	enum class RayTypes : UINT
	{
		Primary = 0u,
		Shadow,

		Count
	};

	void Initialize(D_SERIALIZATION::Json const& settings);
	void Shutdown();
	void Update(D_GRAPHICS::CommandContext& context);
	void UpdateAS(D_GRAPHICS::CommandContext& context);
	void Render(std::wstring const& jobId, SceneRenderContext& renderContext, std::function<void()> postAntiAliasing);

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options);
#endif

}