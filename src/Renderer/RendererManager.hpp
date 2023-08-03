#pragma once

#include "Renderer/RendererCommon.hpp"

#include <Core/Serialization/Json.hpp>
#include <ResourceManager/Resource.hpp>

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER


namespace Darius::Renderer
{

	void Initialize(D_SERIALIZATION::Json const& settings);
	void Shutdown();

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options);
#endif

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

	D_RESOURCE::ResourceHandle GetDefaultGraphicsResource(DefaultResource type);

}