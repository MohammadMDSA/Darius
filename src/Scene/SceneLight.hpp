#pragma once

#include "Scene.hpp"
#include "EntityComponentSystem/Components/LightComponent.hpp"

#ifndef D_SCENE_LIGHT
#define D_SCENE_LIGHT Scene::Lights
#endif // !D_SCENE_LIGHT

namespace Scene::Lights
{
	INLINE void Update(float dt)
	{
		auto& reg = D_WORLD::GetRegistry();
		reg.each([dt](D_ECS_COMP::LightComponent& comp)
			{
				comp.Update(dt);
			});
	}
}