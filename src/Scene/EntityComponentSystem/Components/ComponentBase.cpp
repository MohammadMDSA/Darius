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

#if _D_EDITOR
	void ComponentBase::Copy(bool maintainContext, D_SERIALIZATION::Json& serialized) const
	{
		D_SERIALIZATION::Json data;
		Serialize(data);
		serialized = D_SERIALIZATION::Json();
		serialized["Type"] = "Component";
		serialized["Variation"] = GetComponentName().string();
		Serialize(serialized["Data"]);
	}

	bool ComponentBase::IsCopyableValid() const
	{
		auto go = GetGameObject();
		if(!go || !go->IsValid())
			return false;

		return go->HasComponent(GetComponentName());
	}
#endif

	void ComponentBase::Serialize(D_SERIALIZATION::Json& json) const
	{
		D_SERIALIZATION::Serialize(*this, json);
	}

	void ComponentBase::Deserialize(D_SERIALIZATION::Json const& json)
	{
		D_SERIALIZATION::Deserialize(*this, json);
	}
}