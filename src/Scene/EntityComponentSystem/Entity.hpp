#pragma once

#include <flecs.h>


#ifndef D_ECS
#define D_ECS Darius::Scene::ECS
#endif // !D_ECS

namespace Darius::Scene::ECS
{
	using EntityId = flecs::entity_t;
	using Entity = flecs::entity;
	using ComponentEntry = flecs::untyped_component;
	using ECSRegistry = flecs::world;
}
