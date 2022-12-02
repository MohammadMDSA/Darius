#include "Physics/pch.hpp"
#include "ColliderComponent.hpp"

#include "RigidbodyComponent.hpp"
#include "Physics/PhysicsScene.hpp"

#include <ResourceManager/ResourceManager.hpp>

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
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Physics Material
		{
			D_H_DETAILS_DRAW_PROPERTY("Material");
			D_H_RESOURCE_SELECTION_DRAW_DEF(PhysicsMaterialResource, Material)
		}


		D_H_DETAILS_DRAW_END_TABLE();

		if (valueChanged)
			mChangeSignal();
		return valueChanged;

	}

	void ColliderComponent::Start()
	{
		if (!mMaterialHandle.IsValid())
			SetMaterial(D_PHYSICS::GetDefaultMaterial());
		else
			SetMaterial(mMaterialHandle);
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

	void ColliderComponent::Serialize(D_SERIALIZATION::Json& json) const
	{
		if (mMaterial.IsValid())
			json["Material"] = D_CORE::ToString(mMaterial->GetUuid());
	}

	void ColliderComponent::Deserialize(D_SERIALIZATION::Json const& json)
	{
		D_H_DESERIALIZE_REF_PROP(Material);
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
