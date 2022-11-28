#include "Physics/pch.hpp"
#include "ColliderComponent.hpp"

#include "Physics/Components/RigidBodyComponent.hpp"
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
		D_PHYSICS::PhysicsScene::AddShapeToActor(GetGameObject(), GetPhysicsGeometry(), mDynamic);
	}

	void ColliderComponent::PreUpdate(bool simulating)
	{
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
			D_PHYSICS::PhysicsScene::RemoveShapeFromActor(GetGameObject(), mShape);
			mShape = nullptr;
		}
	}

	void ColliderComponent::OnActivate()
	{
		if (!mShape)
			mShape = D_PHYSICS::PhysicsScene::AddShapeToActor(GetGameObject(), GetPhysicsGeometry(), mDynamic);
	}

	void ColliderComponent::OnDeactivate()
	{
		if (mShape)
		{
			D_PHYSICS::PhysicsScene::RemoveShapeFromActor(GetGameObject(), mShape);
			mShape = nullptr;
		}
	}

}
