#include "Physics/pch.hpp"
#include "CharacterControllerComponent.hpp"

#include "Physics/Resources/PhysicsMaterialResource.hpp"
#include "Physics/PhysicsManager.hpp"

#include <Debug/DebugDraw.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

#define MIN_CONTACT_OFFSET 0.01f

#if _D_EDITOR
#include <ResourceManager/ResourceDragDropPayload.hpp>
#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>
#endif // _D_EDITOR

#include <CharacterControllerComponent.sgenerated.hpp>

using namespace physx;

namespace Darius::Physics
{
	D_H_COMP_DEF(CharacterControllerComponent);

	PxCapsuleClimbingMode::Enum GetPxClimbingMode(CharacterControllerComponent::ClimbingMode mode)
	{
		switch (mode)
		{
		case CharacterControllerComponent::ClimbingMode::Easy:
			return PxCapsuleClimbingMode::eEASY;
		case CharacterControllerComponent::ClimbingMode::Constrained:
			return PxCapsuleClimbingMode::eCONSTRAINED;
		default:
			return PxCapsuleClimbingMode::eEASY;
		}
	}

	PxControllerNonWalkableMode::Enum GetPxNonWalkableMode(CharacterControllerComponent::NonWalkableMode mode)
	{
		switch (mode)
		{
		default:
		case CharacterControllerComponent::NonWalkableMode::PreventClimbing:
			return PxControllerNonWalkableMode::ePREVENT_CLIMBING;
		case CharacterControllerComponent::NonWalkableMode::PreventClimbingAndForceSliding:
			return PxControllerNonWalkableMode::ePREVENT_CLIMBING_AND_FORCE_SLIDING;
		}
	}

	PxCapsuleControllerDesc GetCapsuleDesc(CharacterControllerComponent* ccc)
	{
		PxCapsuleControllerDesc result;
		result.radius = ccc->GetRadius();
		result.height = ccc->GetHeight();
		result.climbingMode = GetPxClimbingMode(ccc->GetClimbingMode());
		result.nonWalkableMode = GetPxNonWalkableMode(ccc->GetNonWalkableMode());
		result.contactOffset = ccc->GetContactOffset();
		auto pos = ccc->GetTransform()->GetPosition();
		result.position = PxExtendedVec3(pos.GetX(), pos.GetY(), pos.GetZ());
		result.slopeLimit = D_MATH::Cos(D_MATH::Deg2Rad(ccc->GetSlopeLimit()));
		result.stepOffset = ccc->GetStepOffset();
		result.upDirection = GetVec3(ccc->GetUpDirection());

		auto material = ccc->GetPhysicsMaterial();
		if (material && material->IsLoaded())
			result.material = const_cast<physx::PxMaterial*>(material->GetMaterial());

		return result;
	}

	CharacterControllerComponent::CharacterControllerComponent() :
		Super(),
		mHeight(0.f),
		mRadius(0.f),
		mClimbingMode(ClimbingMode::Easy),
		mNonWalkableMode(NonWalkableMode::PreventClimbing),
		mSlopeLimit(0.707f),
		mContactOffset(0.1f),
		mStepOffset(0.5f),
		mUpDirection(D_MATH::Vector3::Up),
		mPhysicsMaterial(nullptr) {}

	CharacterControllerComponent::CharacterControllerComponent(D_CORE::Uuid const& uuid) :
		Super(uuid),
		mHeight(0.f),
		mRadius(0.f),
		mClimbingMode(ClimbingMode::Easy),
		mNonWalkableMode(NonWalkableMode::PreventClimbing),
		mSlopeLimit(0.707f),
		mContactOffset(0.1f),
		mStepOffset(0.5f),
		mUpDirection(D_MATH::Vector3::Up),
		mPhysicsMaterial(nullptr) {}

	void CharacterControllerComponent::Awake()
	{
		Super::Awake();

		if (!mPhysicsMaterial.IsValid())
			mPhysicsMaterial = D_RESOURCE::GetResourceSync<PhysicsMaterialResource>(D_PHYSICS::GetDefaultMaterial());

		mController = static_cast<PxCapsuleController*>(D_PHYSICS::GetScene()->CreateController(GetCapsuleDesc(this)));
	}

	void CharacterControllerComponent::OnDestroy()
	{
		mController->release();

		Super::OnDestroy();
	}

	void CharacterControllerComponent::SetHeight(float height)
	{
		height = D_MATH::Max(0.f, height);

		if (mHeight == height)
			return;

		mHeight = height;

		if (mController)
			mController->setHeight(height);

		mChangeSignal(this);
	}

	void CharacterControllerComponent::SetRadius(float radius)
	{
		radius = D_MATH::Max(0.f, radius);

		if (mRadius == radius)
			return;

		mRadius = radius;

		if (mController)
			mController->setRadius(radius);

		mChangeSignal(this);
	}

	void CharacterControllerComponent::SetClimbingMode(ClimbingMode mode)
	{
		if (mClimbingMode == mode)
			return;

		mClimbingMode = mode;

		if (mController)
			mController->setClimbingMode(GetPxClimbingMode(mClimbingMode));

		mChangeSignal(this);
	}

	void CharacterControllerComponent::SetNonWalkableMode(NonWalkableMode mode)
	{
		if (mNonWalkableMode == mode)
			return;

		mNonWalkableMode = mode;

		if (mController)
			mController->setNonWalkableMode(GetPxNonWalkableMode(mNonWalkableMode));

		mChangeSignal(this);
	}

	void CharacterControllerComponent::SetSlopeLimit(float limit)
	{
		limit = D_MATH::Clamp(limit, 0.f, 90.f);
		
		if (mSlopeLimit == limit)
			return;

		mSlopeLimit = limit;

		if (mController)
			mController->setSlopeLimit(D_MATH::Cos(D_MATH::Deg2Rad(limit)));

		mChangeSignal(this);
	}

	void CharacterControllerComponent::SetContactOffset(float offset)
	{
		offset = D_MATH::Max(MIN_CONTACT_OFFSET, offset);

		if (offset == mContactOffset)
			return;

		mContactOffset = offset;

		if (mController)
			mController->setContactOffset(offset);

		mChangeSignal(this);
	}

	void CharacterControllerComponent::SetStepOffset(float offset)
	{
		offset = D_MATH::Max(0.f, offset);
		
		if (offset == mStepOffset)
			return;

		mStepOffset = offset;
		
		if (mController)
			mController->setStepOffset(mStepOffset);

		mChangeSignal(this);
	}

	void CharacterControllerComponent::SetUpDirection(D_MATH::Vector3 const& up)
	{
		D_ASSERT(up.Length() > 0);
		auto upVec = up.Normal();

		if (mUpDirection == upVec)
			return;

		mUpDirection = upVec;

		if (mController)
			mController->setUpDirection(GetVec3(upVec));

		mChangeSignal(this);
	}

	void CharacterControllerComponent::SetPhysicsMaterial(PhysicsMaterialResource* material)
	{
		if (mPhysicsMaterial == material)
			return;

		mPhysicsMaterial = material;

		if (!mPhysicsMaterial.IsValid())
			mPhysicsMaterial = D_RESOURCE::GetResourceSync<PhysicsMaterialResource>(D_PHYSICS::GetDefaultMaterial());

		D_ASSERT(mPhysicsMaterial.IsValid());

		if (!mPhysicsMaterial->IsLoaded())
			D_RESOURCE_LOADER::LoadResourceAsync(mPhysicsMaterial.Get(), nullptr, true);
	}

#if _D_EDITOR
	bool CharacterControllerComponent::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Height
		{
			auto value = GetHeight();
			D_H_DETAILS_DRAW_PROPERTY("Capsule Height");
			if (ImGui::DragFloat("##Height", &value, 0.1f, 0.f, FLT_MAX, "%.1f"))
			{
				SetHeight(value);
				valueChanged = true;
			}
		}

		// Radius
		{
			auto value = GetRadius();
			D_H_DETAILS_DRAW_PROPERTY("Capsule Radius");
			if (ImGui::DragFloat("##Radius", &value, 0.1f, 0.f, FLT_MAX, "%.1f"))
			{
				SetRadius(value);
				valueChanged = true;
			}
		}

		// Climbing Mode
		{
			D_H_DETAILS_DRAW_PROPERTY("Climbing Mode");
			D_H_DETAILS_DRAW_ENUM_SELECTION_SIMPLE(ClimbingMode, ClimbingMode);
		}

		// Non Walkable Mode
		{
			D_H_DETAILS_DRAW_PROPERTY("Non Walkable Mode");
			D_H_DETAILS_DRAW_ENUM_SELECTION_SIMPLE(NonWalkableMode, NonWalkableMode);
		}

		// Slope Limit
		{
			D_H_DETAILS_DRAW_PROPERTY("Slope Limit");
			auto value = GetSlopeLimit();
			if (ImGui::SliderFloat("##SlopeLimit", &value, 0.f, 90.f, "%.1f"))
			{
				SetSlopeLimit(value);
				valueChanged = true;
			}
		}

		// Contact Offset
		{
			D_H_DETAILS_DRAW_PROPERTY("Contact Offset");
			auto value = GetContactOffset();
			if (ImGui::DragFloat("##ContactOffset", &value, 0.1f, MIN_CONTACT_OFFSET, FLT_MAX, "%.2f"))
			{
				SetContactOffset(value);
				valueChanged = true;
			}
		}

		// Step Offset
		{
			D_H_DETAILS_DRAW_PROPERTY("Step Offset");
			auto value = GetStepOffset();
			if (ImGui::DragFloat("##StepOffset", &value, 0.1f, 0.f, FLT_MAX, "%.2f"))
			{
				SetStepOffset(value);
				valueChanged = true;
			}
		}

		// Up Direcrion
		{
			D_H_DETAILS_DRAW_PROPERTY("Up Direction");
			auto value = GetUpDirection();
			if (D_MATH::DrawDetails(value, D_MATH::Vector3::Up))
			{
				SetUpDirection(value);
				valueChanged = true;
			}
		}

		// Physics Material
		{
			D_H_DETAILS_DRAW_PROPERTY("Physics Material");
			D_H_RESOURCE_SELECTION_DRAW_DEF(PhysicsMaterialResource, PhysicsMaterial);
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}

	void CharacterControllerComponent::OnGizmo() const
	{
		D_DEBUG_DRAW::DrawCapsule(GetTransform()->GetPosition(), GetRadius() + GetContactOffset(), GetHeight() / 2, D_MATH::Quaternion::Identity, D_DEBUG_DRAW::CapsuleOrientation::AlongY);
	}

#endif // _D_EDITOR

}
