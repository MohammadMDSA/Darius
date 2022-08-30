#include "Scene/pch.hpp"
#include "ComponentBase.hpp"
#include "TransformComponent.hpp"

using namespace D_CORE;

namespace Darius::Scene::ECS::Components
{
	D_H_COMP_DEF(ComponentBase);

	ComponentBase::ComponentBase() :
		mUuid(GenerateUuid()),
		mStarted(false),
		mEnabled(true),
		mGameObject(nullptr)
	{}

	ComponentBase::ComponentBase(Uuid uuid) :
		mUuid(uuid),
		mStarted(false),
		mEnabled(true),
		mGameObject(nullptr)
	{}

}