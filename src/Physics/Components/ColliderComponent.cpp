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
		mTrigger(false),
		mUsedScale(D_MATH::kOne),
		mCenterOffset(0.f),
		mScaledCenterOffset(0.f)
	{
	}

	ColliderComponent::ColliderComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid),
		mDynamic(false),
		mMaterial(),
		mTrigger(false),
		mUsedScale(D_MATH::kOne),
		mCenterOffset(0.f),
		mScaledCenterOffset(0.f)
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

		// Center offset
		{
			D_H_DETAILS_DRAW_PROPERTY("Center Offset");
			auto offset = GetCenterOffset();
			float drawParams[] = D_H_DRAW_DETAILS_MAKE_VEC_PARAM(0.f, false);
			if (D_MATH::DrawDetails(offset, drawParams))
			{
				SetCenterOffset(offset);
				valueChanged = true;
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;

	}

#endif

	void ColliderComponent::Awake()
	{
		CalculateScaledParameters();
		UpdateGeometry();

		if (!mMaterial.IsValid())
			SetMaterial(static_cast<PhysicsMaterialResource*>(D_RESOURCE::GetRawResourceSync(D_PHYSICS::GetDefaultMaterial())));

		if (!mActor)
			InvalidatePhysicsActor();
	}

	void ColliderComponent::CalculateScaledParameters()
	{
		auto scale = GetTransform()->GetScale();

		// Calc scaled center offset
		{
			mScaledCenterOffset = scale * GetCenterOffset();
		}

		mUsedScale = scale;
	}


	void ColliderComponent::PreUpdate(bool simulating)
	{
		if (!mShape)
			return;

		if (simulating)
		{
			if (IsDynamic() != GetGameObject()->HasComponent<RigidbodyComponent>())
				InvalidatePhysicsActor();

			// Update rot pos
			if (!IsDynamic() && mActor) // Dynamic objects are handled by their rigidbody component
			{
				auto trans = GetTransform();
				auto rot = trans->GetRotation() * GetBiasedRotation();
				auto pos = trans->GetPosition() + (rot * GetScaledCenterOffset());
				mActor->GetPxActor()->setGlobalPose(physx::PxTransform(D_PHYSICS::GetVec3(pos), D_PHYSICS::GetQuat(rot)));
			}
		}

		if (!IsDynamic() && simulating)
			return;

		// Updating shape
		bool geomChanged = false;
		auto geom = UpdateAndGetPhysicsGeometry(geomChanged);

		if (geomChanged && geom)
		{
			mShape->setGeometry(*geom);
			auto rot = GetTransform()->GetRotation() * GetBiasedRotation();
			auto offset = GetScaledCenterOffset();
			
			if (!offset.IsZero())
			{
				physx::PxTransform trans(D_PHYSICS::GetVec3(GetScaledCenterOffset()));
				mShape->setLocalPose(trans);
			}
			SetClean();
		}

	}

	void ColliderComponent::OnPreDestroy()
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

		// Apply shape offset
		auto offset = GetCenterOffset();
		if (!offset.IsZero())
		{
			physx::PxTransform trans(D_PHYSICS::GetVec3(GetScaledCenterOffset()));
			mShape->setLocalPose(trans);
		}

		if (!mShape)
			return;

		if (trigger)
		{
			mShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
			mShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
		}
		else
		{
			mShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
			mShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
		}
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
			if (trigger)
			{
				mShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
				mShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
			}
			else
			{
				mShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
				mShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
			}
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

	void ColliderComponent::SetCenterOffset(D_MATH::Vector3 const& centerOffset)
	{
		if (mCenterOffset == centerOffset)
			return;

		mCenterOffset = centerOffset;
		SetDirty();
		mChangeSignal(this);
	}

	physx::PxGeometry const* ColliderComponent::UpdateAndGetPhysicsGeometry(bool& changed)
	{
		auto scale = GetTransform()->GetScale();
		if (!IsDirty() && GetUsedScale().NearEquals(scale, COLLIDER_SCALE_TOLERANCE))
		{
			changed = false;
			return GetPhysicsGeometry();
		}

		changed = true;

		CalculateScaledParameters();
		if (!UpdateGeometry())
		{
			SetDirty();
			return nullptr;
		}

		return GetPhysicsGeometry();
	}
}
