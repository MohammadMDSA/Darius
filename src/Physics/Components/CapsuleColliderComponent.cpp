#include "Physics\pch.hpp"
#include "CapsuleColliderComponent.hpp"

#include <Debug/DebugDraw.hpp>

#ifdef _D_EDITOR
#include <imgui.h>
#endif

#include "CapsuleColliderComponent.sgenerated.hpp"

namespace Darius::Physics
{

	D_H_COMP_DEF(CapsuleColliderComponent);

	CapsuleColliderComponent::CapsuleColliderComponent() :
		ColliderComponent(),
		mRadius(0.5f),
		mHalfHeight(0.5f)
	{
		SetDirty();
	}

	CapsuleColliderComponent::CapsuleColliderComponent(D_CORE::Uuid const& uuid) :
		ColliderComponent(uuid),
		mRadius(0.5f),
		mHalfHeight(0.5f)
	{
		SetDirty();
	}

#ifdef _D_EDITOR

	bool CapsuleColliderComponent::DrawDetails(float params[])
	{
		bool valueChanged = false;

		valueChanged |= ColliderComponent::DrawDetails(params);

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Radius
		{
			float radius = GetRadius();
			D_H_DETAILS_DRAW_PROPERTY("Radius");
			if (ImGui::DragFloat("##Radius", &radius, 0.1f))
			{
				SetRadius(radius);
				valueChanged = true;
			}
		}

		// Half height
		{
			float halfHeight = GetHalfHeight();
			D_H_DETAILS_DRAW_PROPERTY("Half Height");
			if (ImGui::DragFloat("##HalfHeight", &halfHeight, 0.1f))
			{
				SetHalfHeight(halfHeight);
				valueChanged = true;
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}

	void CapsuleColliderComponent::OnGizmo() const
	{
		if (!IsActive())
			return;

		auto trans = GetTransform();
		auto rot = trans->GetRotation();
		auto offset = rot * GetScaledCenterOffset();
		D_DEBUG_DRAW::DrawCapsule(trans->GetPosition() + offset, mScaledRadius, mScaledHalfHeight, rot, D_DEBUG_DRAW::CapsuleOrientation::AlongX, 16, 0., { 0.f, 1.f, 0.f, 1.f });
	}

#endif

	void CapsuleColliderComponent::SetRadius(float radius)
	{
		if (mRadius == radius)
			return;

		mRadius = radius;
		SetDirty();

		mChangeSignal(this);
	}

	void CapsuleColliderComponent::SetHalfHeight(float halfHeight)
	{
		if (mHalfHeight == halfHeight)
			return;

		mHalfHeight = halfHeight;
		SetDirty();

		mChangeSignal(this);
	}

	void CapsuleColliderComponent::CalculateScaledParameters()
	{
		auto scale = GetTransform()->GetScale();

		mScaledRadius = GetRadius();
		mScaledHalfHeight = GetHalfHeight();

		mScaledHalfHeight *= D_MATH::Abs(scale.GetX());
		mScaledRadius *= D_MATH::Max(D_MATH::Abs(scale.GetY()), D_MATH::Abs(scale.GetZ()));

		mScaledHalfHeight = D_MATH::Max(MinHalfHeight, mScaledHalfHeight);
		mScaledRadius = D_MATH::Max(MinRadius, mScaledRadius);

		Super::CalculateScaledParameters();
	}

	bool CapsuleColliderComponent::CalculateGeometry(_OUT_ physx::PxGeometry& geom) const
	{
		physx::PxCapsuleGeometry& caps = reinterpret_cast<physx::PxCapsuleGeometry&>(geom);
		caps = physx::PxCapsuleGeometry(mScaledRadius, mScaledHalfHeight);

		return true;
	}

}
