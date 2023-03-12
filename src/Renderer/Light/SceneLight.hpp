#pragma once

#include "Renderer/Components/LightComponent.hpp"

#include <Scene/Scene.hpp>

#ifndef D_LIGHT
#define D_LIGHT Darius::Renderer::LightManager
#endif // !D_LIGHT

namespace Darius::Renderer::LightManager
{
	INLINE void Update(float dt)
	{
		auto& reg = D_WORLD::GetRegistry();
		reg.each([dt](D_GRAPHICS::LightComponent& comp)
			{
				comp.Update(dt);
			});
	}
}