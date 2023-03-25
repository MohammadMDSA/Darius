#pragma once

#include "Renderer/CommandContext.hpp"
#include "Renderer/FrameResource.hpp"

#include <Core/Containers/Vector.hpp>
#include <Math/VectorMath.hpp>

#ifndef D_LIGHT
#define D_LIGHT Darius::Renderer::LightManager
#endif // !D_LIGHT

namespace Darius::Renderer
{
	class MeshSorter;
}

namespace Darius::Renderer::LightManager
{
	constexpr UINT		MaxNumDirectionalLight = 6;
	constexpr UINT		MaxNumPointLight = 125;
	constexpr UINT		MaxNumSpotLight = 125;
	constexpr UINT		MaxNumLight = MaxNumDirectionalLight + MaxNumPointLight + MaxNumSpotLight;

	enum class LightSourceType
	{
		DirectionalLight,
		PointLight,
		SpotLight
	};

	struct LightData
	{
		DirectX::XMFLOAT3	Color = D_MATH::Vector3(D_MATH::kOne);
		DirectX::XMFLOAT3	Direction = { 0.f, 0.f, -1.f };				// Directional/Spot light only
		DirectX::XMFLOAT3	Position = D_MATH::Vector3(D_MATH::kZero);  // Point light only
		float				Intencity = 1.f;							// Point/Spot light only
		float				Range = 10.f;								// Point/Spot light only
		DirectX::XMFLOAT2	SpotAngles = { 1000.f, 0.8f };				// Spot light only
		DirectX::XMFLOAT4X4	ShadowMatrix;
		bool				CastsShadow = true;
		DirectX::XMFLOAT3	padding;
	};

	void				Initialize();
	void				Shutdown();
	void				Reset();

	void				UpdateBuffers(D_GRAPHICS::GraphicsContext& context);
	void				RenderShadows(D_CONTAINERS::DVector<D_RENDERER_FRAME_RESOURCE::RenderItem> const& shadowRenderItems);

	D3D12_CPU_DESCRIPTOR_HANDLE GetLightMaskHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE GetLightDataHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE GetShadowTextureArrayHandle();

	// Accessed by light components
	/// O(n) Don't use too often
	int					AccuireLightSource(LightSourceType type);
	/// O(n) Don't use too often
	int					SwapLightSource(LightSourceType type, LightSourceType preType, int preIndex);
	void				ReleaseLight(LightSourceType preType, int preIndex);

	void				UpdateLight(LightSourceType type, int index, D_MATH::Transform const& trans, bool active, LightData const& light);


	D_H_SERIALIZE_ENUM(LightSourceType,
		{
			{ LightSourceType::DirectionalLight, "DirectionalLight" },
			{ LightSourceType::PointLight, "PointLight" },
			{ LightSourceType::SpotLight, "SpotLight" }
		});
}
