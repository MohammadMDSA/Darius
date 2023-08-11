#include "Renderer/pch.hpp"
#include "RayTracingLightContext.hpp"

#include "Renderer/Components/LightComponent.hpp"

namespace Darius::Renderer::RayTracing::Light
{
	void RayTracingLightContext::Update(D_MATH_CAMERA::Camera const& viewerCamera)
	{

		Reset();

		D_WORLD::IterateComponents<D_RENDERER::LightComponent>([&](D_RENDERER::LightComponent& comp)
			{
				if (!comp.IsActive())
					return;

				auto lightType = comp.GetLightType();
				auto& lightData = AddLightSource(comp.GetLightData(), lightType);
				auto trans = comp.GetTransform();
				lightData.Position = (DirectX::XMFLOAT3)trans->GetPosition();
				lightData.Direction = (DirectX::XMFLOAT3)trans->GetRotation().GetForward();
			});

	}
}

