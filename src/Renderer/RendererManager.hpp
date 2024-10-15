#pragma once

#include "RendererCommon.hpp"
#include "Resources/TextureResource.hpp"

#include <Core/Serialization/Json.hpp>
#include <Math/Bounds/DynamicBVH.hpp>
#include <ResourceManager/Resource.hpp>
#include <Scene/EntityComponentSystem/CompRef.hpp>

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	enum class RendererType
	{
		Rasterization,
		RayTracing
	};

	enum class DefaultResource
	{
		// Meshes
		BoxMesh,
		CylinderMesh,
		GeosphereMesh,
		QuadMesh,
		SphereMesh,
		LowPolySphereMesh,
		LineMesh,
		GridPatch2x2Mesh,
		GridPatch4x4Mesh,
		GridPatch8x8Mesh,
		GridPatch16x16Mesh,
		Grid100x100Mesh,

		// Materials
		Material,

		// Textures
		Texture2DMagenta,
		Texture2DBlackOpaque,
		Texture2DBlackTransparent,
		Texture2DWhiteOpaque,
		Texture2DWhiteTransparent,
		Texture2DNormalMap,
		TextureCubeMapBlack
	};

	void						Initialize(D_SERIALIZATION::Json const& settings);
	void						Shutdown();
	void						Update();
	void						Render(std::wstring const& jobId, SceneRenderContext& renderContext, std::function<void()> postAntiAliasing = nullptr);

#ifdef _D_EDITOR
	bool						OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options);
#endif

	RendererType				GetActiveRendererType();

	// Allocating from heaps
	D_GRAPHICS_MEMORY::DescriptorHandle AllocateTextureDescriptor(UINT count = 1);
	D_GRAPHICS_MEMORY::DescriptorHandle AllocateSamplerDescriptor(UINT count = 1);
	uint32_t					GetTextureDescriptorHeapDescriptorSize();
	uint32_t					GetSamplerDescriptorHeapDescriptorSize();
	void						SetCbvSrvUavDescriptorHeap(D_GRAPHICS::CommandContext& context);
	void						SetSamplerDescriptorHeap(D_GRAPHICS::CommandContext& context);

	// Set IBL properties
	void						SetIBLTextures(D_RENDERER::TextureResource* diffuseIBL, D_RENDERER::TextureResource* specularIBL);
	void						SetIBLBias(float LODBias);

	// Set render options
	void						SetForceWireframe(bool val);

	D_RESOURCE::ResourceHandle	GetDefaultGraphicsResource(DefaultResource type);

	D_MATH_BOUNDS::DynamicBVH<D_ECS::UntypedCompRef>::ID RegisterComponent(D_ECS::UntypedCompRef const& compRef);
	bool						UpdateComponentBounds(D_MATH_BOUNDS::DynamicBVH<D_ECS::UntypedCompRef>::ID const& id, D_MATH_BOUNDS::Aabb const& aabb);
	void						UnregisterComponent(D_MATH_BOUNDS::DynamicBVH<D_ECS::UntypedCompRef>::ID const& id);

	D_MATH_BOUNDS::DynamicBVH<D_ECS::UntypedCompRef> const& GetSceneBvh();
}