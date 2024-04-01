#include "Scene/pch.hpp"
#include "ComponentBase.hpp"

#include "TransformComponent.hpp"
#include "Scene/GameObject.hpp"

#include "ComponentBase.sgenerated.hpp"

using namespace D_CORE;

namespace Darius::Scene::ECS::Components
{
	D_H_COMP_DEF(ComponentBase);

#if _D_EDITOR
	D_CORE::Signal<void(D_FILE::Path const&, Darius::ResourceManager::ResourceHandle const&, bool select)> ComponentBase::RequestPathChange;
#endif // _D_EDITOR

	ComponentBase::ComponentBase() :
		mUuid(GenerateUuid()),
		mStarted(false),
		mEnabled(true),
		mGameObject(nullptr),
		mDestroyed(false),
		mDirty(false),
		mChangeSignal()
	{}

	ComponentBase::ComponentBase(Uuid const& uuid) :
		mUuid(uuid),
		mStarted(false),
		mEnabled(true),
		mGameObject(nullptr),
		mDestroyed(false),
		mDirty(false),
		mChangeSignal()
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

	bool ComponentBase::IsActive() const
	{
		return mGameObject->IsActive() && mGameObject->IsInScene() && mEnabled;
	}

}