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
		mMaterial(),
		mTrigger(false)
	{
	}

	ColliderComponent::ColliderComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid),
		mDynamic(false),
		mMaterial(),
		mTrigger(false)
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

		// Is Trigger
		{
			D_H_DETAILS_DRAW_PROPERTY("Trigger");
			bool trigger = IsTrigger();
			if (ImGui::Checkbox("##Trigger", &trigger))
			{
				SetTrigger(trigger);
				valueChanged;
			}

		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;

	}
#endif

	void ColliderComponent::Awake()
	{
		if (!mMaterial.IsValid())
			SetMaterial(static_cast<PhysicsMaterialResource*>(D_RESOURCE::GetRawResourceSync(D_PHYSICS::GetDefaultMaterial())));

		InvalidatePhysicsActor();
	}

	void ColliderComponent::PreUpdate(bool simulating)
	{
		if (!mShape)
			return;

		if (!simulating && IsDynamic() != GetGameObject()->HasComponent<RigidbodyComponent>())
			InvalidatePhysicsActor();

		// Update rot pos
		if (!IsDynamic()) // Dynamic objects are handled by their rigidbody component
		{
			auto trans = GetTransform();
			auto pos = trans->GetPosition();
			auto rot = trans->GetRotation() * GetBiasedRotation();
			mActor->GetPxActor()->setGlobalPose(physx::PxTransform(D_PHYSICS::GetVec3(pos), D_PHYSICS::GetQuat(rot)));
		}

		if (!IsDynamic() && simulating)
			return;

		// Updating shape
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
			D_PHYSICS::GetScene()->RemoveCollider(this);
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
			D_PHYSICS::GetScene()->RemoveCollider(this);
			mShape = nullptr;
		}
	}

	void ColliderComponent::InvalidatePhysicsActor()
	{
		if (IsDestroyed())
			return;

		mShape = nullptr;

		bool trigger = IsTrigger();
		auto material = D_PHYSICS::GetDefaultMaterial();
		mShape = D_PHYSICS::GetScene()->AddCollider(this, mDynamic, &mActor);
		mShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, trigger);
		mShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !trigger);
	}

	void ColliderComponent::SetMaterial(PhysicsMaterialResource* material)
	{
		if (mMaterial == material)
			return;

		mMaterial = material;

		if (!mMaterial.IsValid())
			mMaterial = D_RESOURCE::GetResourceSync<PhysicsMaterialResource>(D_PHYSICS::GetDefaultMaterial());

		if (mMaterial->IsLoaded())
			ReloadMaterialData();
		else
		{
			D_RESOURCE_LOADER::LoadResourceAsync(material, [&](auto material)
				{
					ReloadMaterialData();
				}, true);

		}

		mChangeSignal(this);
	}

	void ColliderComponent::SetTrigger(bool trigger)
	{
		if (mTrigger == trigger)
			return;

		mTrigger = trigger;

		if (mShape)
		{
			auto trigger = IsTrigger();
			mShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, trigger);
			mShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !trigger);
		}
		
		mChangeSignal(this);

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
