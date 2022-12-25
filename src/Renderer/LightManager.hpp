#pragma once

#include "CommandContext.hpp"

#include <Core/Containers/Vector.hpp>
#include <Math/VectorMath.hpp>

#ifndef D_LIGHT
#define D_LIGHT Darius::Renderer::LightManager
#endif // !D_LIGHT

using namespace D_CONTAINERS;
using namespace D_MATH;

namespace Darius::Renderer
{
	class MeshSorter;
}

namespace Darius::Renderer::LightManager
{
	constexpr int		MaxNumDirectionalLight = 6;
	constexpr int		MaxNumPointLight = 125;
	constexpr int		MaxNumSpotLight = 125;
	constexpr int		MaxNumLight = MaxNumDirectionalLight + MaxNumPointLight + MaxNumSpotLight;

	enum class LightSourceType
	{
		DirectionalLight,
		PointLight,
		SpotLight
	};

	struct LightData
	{
		XMFLOAT3		Color = Vector3(kOne);
		XMFLOAT3		Direction = { 0.f, 0.f, -1.f };// Directional/Spot light only
		XMFLOAT3		Position = Vector3(kZero);  // Point light only
		float			Intencity = 1.f;			// Point/Spot light only
		float			Range = 10.f;			// Point/Spot light only
		XMFLOAT2		SpotAngles = { 1000.f, 0.8};// Spot light only
		XMFLOAT4X4		ShadowMatrix;
		bool			CastsShadow = true;
	};

	void				Initialize();
	void				Shutdown();
	void				Reset();

	void				UpdateBuffers(D_GRAPHICS::GraphicsContext& context);
	void				RenderShadows(Darius::Renderer::MeshSorter* parentSorter);

	D3D12_CPU_DESCRIPTOR_HANDLE GetLightMaskHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE GetLightDataHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE GetShadowTextureArrayHandle();

	// Accessed by light components
	/// O(n) Don't use too often
	int					AccuireLightSource(LightSourceType type);
	/// O(n) Don't use too often
	int					SwapLightSource(LightSourceType type, LightSourceType preType, int preIndex);
	void				ReleaseLight(LightSourceType preType, int preIndex);

	void				UpdateLight(LightSourceType type, int index, Transform const& trans, bool active, LightData const& light);
}
