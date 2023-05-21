#include "Physics/pch.hpp"
#include "ColliderComponent.hpp"

#include "RigidbodyComponent.hpp"
#include "Physics/PhysicsScene.hpp"

#include <ResourceManager/ResourceManager.hpp>

#ifdef _D_EDITOR
#include <Libs/FontIcon/IconsFontAwesome6.h>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "ColliderComponent.sgenerated.hpp"

using namespace physx;

namespace Darius::Physics
{
	D_H_COMP_DEF(ColliderComponent);

	ColliderComponent::ColliderComponent() :
		ComponentBase(),
		mDynamic(false),
		mMaterial(GetAsCountedOwner())
	{
	}

	ColliderComponent::ColliderComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid),
		mDynamic(false),
		mMaterial(GetAsCountedOwner())
	{
	}

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
		if (!mMaterial.IsValid())
			SetMaterial(D_PHYSICS::GetDefaultMaterial());

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

	void ColliderComponent::_SetMaterial(D_RESOURCE::ResourceHandle handle)
	{
		mMaterial = D_RESOURCE::GetResource<PhysicsMaterialResource>(handle, *this);
		ReloadMaterialData();
	}
	
	void ColliderComponent::ReloadMaterialData()
	{
		if (!mShape)
			return;
		physx::PxMaterial* mats[] = { const_cast<physx::PxMaterial*>(mMaterial.Get()->GetMaterial()) };
		mShape->setMaterials(mats, 1);
	}

	void ColliderComponent::OnDeserialized()
	{
		ReloadMaterialData();
	}
}
