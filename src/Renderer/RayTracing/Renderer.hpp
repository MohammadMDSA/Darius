#pragma once

#include "Renderer/RendererCommon.hpp"
#include "RayTracingCommandContext.hpp"

#include <Core/Serialization/Json.hpp>
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
	void Update();
	void Render(std::wstring const& jobId, SceneRenderContext& renderContext, std::function<void()> postAntiAliasing);

	// Allocating from heaps
	D_GRAPHICS_MEMORY::DescriptorHandle AllocateTextureDescriptor(UINT count = 1);
	D_GRAPHICS_MEMORY::DescriptorHandle AllocateSamplerDescriptor(UINT count = 1);

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options);
#endif

}