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
		mMaterial(),
		mTrigger(false),
		mUsedScale(D_MATH::kOne),
		mCenterOffset(0.f),
		mScaledCenterOffset(0.f),
		mActor(nullptr)
	{
	}

	ColliderComponent::ColliderComponent(D_CORE::Uuid const& uuid) :
		ComponentBase(uuid),
		mMaterial(),
		mTrigger(false),
		mUsedScale(D_MATH::kOne),
		mCenterOffset(0.f),
		mScaledCenterOffset(0.f),
		mActor(nullptr)
	{
	}

#ifdef _D_EDITOR

	bool ColliderComponent::DrawDetails(float params[])
	{
		bool valueChanged = false;

		if(IsDirty())
			UpdateShape();

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
			if (D_MATH::DrawDetails(offset, D_MATH::Vector3::Zero))
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
		if (!mMaterial.IsValid())
			SetMaterial(static_cast<PhysicsMaterialResource*>(D_RESOURCE::GetRawResourceSync(D_PHYSICS::GetDefaultMaterial())));

#if _D_EDITOR
		mEditorTransformChangeSignalConnection = GetTransform()->mWorldChanged.ConnectComponent(this, &ThisClass::OnTransformWorldChangedEditor);
#endif // _D_EDITOR

	}

#if _D_EDITOR
	void ColliderComponent::OnTransformWorldChangedEditor(D_MATH::TransformComponent* trans, D_MATH::Transform const& worldTransform)
	{
		if (IsStarted())
			return;

		SetDirty();
		bool changed = false;
		UpdateAndGetPhysicsGeometry(changed);
	}
#endif

	void ColliderComponent::Start()
	{
		CalculateScaledParameters();
		UpdateGeometry();

		if (!mActor.IsValid())
			InvalidatePhysicsActor();
		else
			mActor->AddCollider(this);

		mTransformChangeSignalConnection =
			GetTransform()->mWorldChanged.ConnectComponent(this, &ThisClass::OnTransformWorldChanged);
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

	void ColliderComponent::PreUpdate()
	{
		if(!mActor.IsValid())
		{
			if(!InvalidatePhysicsActor())
				return;
		}

		bool dynamic = mActor->IsDynamic();

		if (!dynamic)
			return;

		// Updating shape
		UpdateShape();

	}

	void ColliderComponent::UpdateShape()
	{
		bool geomChanged = false;
		auto geom = UpdateAndGetPhysicsGeometry(geomChanged);

		if(geomChanged && geom)
		{
			if(mActor.IsValid())
			{
				auto shape = mActor->GetShape(GetComponentName());
				shape->setGeometry(*geom);
				auto offset = GetScaledCenterOffset();
				if(!offset.IsZero())
				{
					physx::PxTransform trans(D_PHYSICS::GetVec3(GetScaledCenterOffset()));
					shape->setLocalPose(trans);
				}
			}
			SetClean();
		}
	}

	void ColliderComponent::OnDestroy()
	{

#if _D_EDITOR
		mEditorTransformChangeSignalConnection.disconnect();
#endif // _D_EDITOR

		mTransformChangeSignalConnection.disconnect();
		if (mActor.IsValid())
		{
			mActor->RemoveCollider(this);
		}
	}

	void ColliderComponent::OnActivate()
	{
		if (mActor.IsValid())
			InvalidatePhysicsActor();
	}

	void ColliderComponent::OnDeactivate()
	{
		if (mActor.IsValid())
		{
			mActor->RemoveCollider(this);
		}
	}

	bool ColliderComponent::InvalidatePhysicsActor()
	{
		if (IsDestroyed())
			return false;

		bool trigger = IsTrigger();
		auto material = D_PHYSICS::GetDefaultMaterial();

		mActor = D_PHYSICS::GetScene()->FindOrCreatePhysicsActor(GetGameObject());
		auto shape = mActor->AddCollider(this);

		if(!shape)
		{
			mActor = nullptr;
			return false;
		}

		// Apply shape offset
		auto offset = GetCenterOffset();
		if (!offset.IsZero())
		{
			physx::PxTransform trans(D_PHYSICS::GetVec3(GetScaledCenterOffset()));
			shape->setLocalPose(trans);
		}

		if (trigger)
		{
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
		}
		else
		{
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
		}

		return true;
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

		if (!mActor.IsValid())
			return;

		auto shape = mActor->GetShape(this->GetComponentName());

		if (shape)
		{
			auto trigger = IsTrigger();
			if (trigger)
			{
				shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
				shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
			}
			else
			{
				shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
				shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
			}
		}

		mChangeSignal(this);

	}

	void ColliderComponent::ReloadMaterialData()
	{
		if (!mActor)
			return;

		auto shape = mActor->GetShape(GetComponentName());

		D_ASSERT(shape);

		physx::PxMaterial* mats[] = { const_cast<physx::PxMaterial*>(mMaterial.Get()->GetMaterial()) };
		shape->setMaterials(mats, 1);
	}

	void ColliderComponent::OnDeserialized()
	{
		ReloadMaterialData();
	}

	void ColliderComponent::SetCenterOffset(D_MATH::Vector3 const& centerOffset)
	{
		if (mCenterOffset.Equals(centerOffset))
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
