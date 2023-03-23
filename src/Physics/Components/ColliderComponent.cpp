#include "Physics/pch.hpp"
#include "ColliderComponent.hpp"

#include "RigidbodyComponent.hpp"
#include "Physics/PhysicsScene.hpp"

#include <ResourceManager/ResourceManager.hpp>
#include <Utils/DragDropPayload.hpp>

#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>

#include "ColliderComponent.sgenerated.hpp"

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

#ifdef _D_EDITOR
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

		return valueChanged;

	}
#endif

	void ColliderComponent::Awake()
	{
		if (!mMaterialHandle.IsValid())
			SetMaterial(D_PHYSICS::GetDefaultMaterial());
		else
			SetMaterial(mMaterialHandle);
		InvalidatePhysicsActor();
	}

	void ColliderComponent::PreUpdate(bool simulating)
	{

		if (!simulating && IsDynamic() != GetGameObject()->HasComponent<RigidbodyComponent>())
			InvalidatePhysicsActor();

		if (!IsDynamic() && simulating)
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
		if (IsDestroyed())
			return;

		mShape = nullptr;

		auto material = D_PHYSICS::GetDefaultMaterial();
		mShape = D_PHYSICS::PhysicsScene::AddCollider(this, mDynamic);
	}

}
