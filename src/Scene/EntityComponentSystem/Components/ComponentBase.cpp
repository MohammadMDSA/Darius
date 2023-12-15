#include "Scene/pch.hpp"
#include "ComponentBase.hpp"

#include "TransformComponent.hpp"
#include "Scene/GameObject.hpp"

#include "ComponentBase.sgenerated.hpp"

using namespace D_CORE;

namespace Darius::Scene::ECS::Components
{
	D_H_COMP_DEF(ComponentBase);

	ComponentBase::ComponentBase() :
		mUuid(GenerateUuid()),
		mStarted(false),
		mEnabled(true),
		mGameObject(nullptr),
		mDestroyed(false),
		mDirty(false)
	{}

	ComponentBase::ComponentBase(Uuid uuid) :
		mUuid(uuid),
		mStarted(false),
		mEnabled(true),
		mGameObject(nullptr),
		mDestroyed(false),
		mDirty(false)
	{}

	void ComponentBase::SetEnable(bool value)
	{
		if (!value && !IsDisableable())
			return;

		auto changed = mEnabled != value;
		mEnabled = value;
		if (!changed)
			return;

		if (value)
			OnActivate();
		else
			OnDeactivate();

		if (!mStarted && IsActive())
		{
			mStarted = true;
			Start();
		}
	}

}