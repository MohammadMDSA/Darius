#include "Scene/pch.hpp"
#include "ComponentBase.hpp"

using namespace D_CORE;

namespace Darius::Scene::ECS::Components
{
	D_H_COMP_DEF(ComponentBase);

	ComponentBase::ComponentBase() :
		mUuid(GenerateUuid()) {}

	ComponentBase::ComponentBase(Uuid uuid) :
		mUuid(uuid) {}

}