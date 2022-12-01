#include "Physics/pch.hpp"
#include "ColliderComponent.hpp"

#include "RigidbodyComponent.hpp"
#include "Physics/PhysicsScene.hpp"

#include <imgui.h>

using namespace physx;

namespace Darius::Physics
{
	D_H_COMP_DEF(ColliderComponent);

	ColliderComponent::ColliderComponent() :
		ComponentBase(),
		mDynamic(false)
	{}

	ColliderComponent::ColliderComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid),
		mDynamic(false)
	{}

	bool ColliderComponent::DrawDetails(float params[])
	{

		return false;

	}

	void ColliderComponent::Start()
	{
		InvalidatePhysicsActor();
	}

	void ColliderComponent::PreUpdate(bool simulating)
	{

		if (!simulating && GetDynamic() != GetGameObject()->HasComponent<RigidbodyComponent>())
			InvalidatePhysicsActor();

		if (!GetDynamic() && simulating)
			return;

		// Updating scale, pos, rot
		bool geomChanged = false;
		auto geom = UpdateAndGetPhysicsGeometry(geomChanged);

		if (geomChanged)
		{
			mShape->setGeometry(*geom);
		}

	}

	void ColliderComponent::OnDestroy()
	{
		if (mShape)
		{
			D_PHYSICS::PhysicsScene::RemoveCollider(this);
			mShape = nullptr;
		}
	}

	void ColliderComponent::OnActivate()
	{
		if (!mShape)
			InvalidatePhysicsActor();
	}

	void ColliderComponent::OnDeactivate()
	{
		if (mShape)
		{
			D_PHYSICS::PhysicsScene::RemoveCollider(this);
			mShape = nullptr;
		}
	}

	void ColliderComponent::InvalidatePhysicsActor()
	{
		if (GetDestroyed())
			return;

		mShape = nullptr;

		auto material = D_PHYSICS::GetDefaultMaterial();
		mShape = D_PHYSICS::PhysicsScene::AddCollider(this, mDynamic);
	}

}
