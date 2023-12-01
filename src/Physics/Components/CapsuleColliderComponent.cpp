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
		mHalfHeight(0.5f),
		mOrientation(CapsuleColliderOrientation::AlongY)
	{
		SetDirty();
	}

	CapsuleColliderComponent::CapsuleColliderComponent(D_CORE::Uuid uuid) :
		ColliderComponent(uuid),
		mRadius(0.5f),
		mHalfHeight(0.5f),
		mOrientation(CapsuleColliderOrientation::AlongY)
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

		// Orientation
		{
			D_H_DETAILS_DRAW_PROPERTY("Orientation");
			int orientation = (int)GetOrientation();
			if (ImGui::Combo("##Orientation", &orientation, "Along X\0Along Y\0Along Z\0\0"))
			{
				SetOrientation((CapsuleColliderOrientation)orientation);
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
		D_DEBUG_DRAW::DrawCapsule(trans->GetPosition(), mScaledRadius, mScaledHalfHeight, trans->GetRotation(), (D_DEBUG_DRAW::CapsuleOrientation)mOrientation, 16, 0., { 0.f, 1.f, 0.f, 1.f });
	}

#endif

	void CapsuleColliderComponent::SetOrientation(CapsuleColliderOrientation orientation)
	{
		if (mOrientation == orientation)
			return;
		
		mOrientation = orientation;
		SetDirty();
	}

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

		switch (mOrientation)
		{
		case Darius::Physics::CapsuleColliderComponent::CapsuleColliderOrientation::AlongX:
			mScaledHalfHeight *= D_MATH::Abs(scale.GetX());
			mScaledRadius *= D_MATH::Max(D_MATH::Abs(scale.GetY()), D_MATH::Abs(scale.GetZ()));
			break;
		case Darius::Physics::CapsuleColliderComponent::CapsuleColliderOrientation::AlongY:
			mScaledHalfHeight *= D_MATH::Abs(scale.GetY());
			mScaledRadius *= D_MATH::Max(D_MATH::Abs(scale.GetX()), D_MATH::Abs(scale.GetZ()));
			break;
		case Darius::Physics::CapsuleColliderComponent::CapsuleColliderOrientation::AlongZ:
			mScaledHalfHeight *= D_MATH::Abs(scale.GetZ());
			mScaledRadius *= D_MATH::Max(D_MATH::Abs(scale.GetY()), D_MATH::Abs(scale.GetX()));
			break;
		default:
			mScaledRadius *= 0;
			mScaledHalfHeight *= 0;
		}

		mScaledHalfHeight = D_MATH::Max(MinHalfHeight, mScaledHalfHeight);
		mScaledRadius = D_MATH::Max(MinRadius, mScaledRadius);

		Super::CalculateScaledParameters();
	}

	void CapsuleColliderComponent::CalculateGeometry(_OUT_ physx::PxGeometry& geom) const
	{
		physx::PxCapsuleGeometry& caps = reinterpret_cast<physx::PxCapsuleGeometry&>(geom);
		caps = physx::PxCapsuleGeometry(mScaledRadius, mScaledHalfHeight);
	}

	D_MATH::Quaternion CapsuleColliderComponent::GetBiasedRotation() const
	{
		switch (mOrientation)
		{
		case Darius::Physics::CapsuleColliderComponent::CapsuleColliderOrientation::AlongX:
			return D_MATH::Quaternion::Identity;
		case Darius::Physics::CapsuleColliderComponent::CapsuleColliderOrientation::AlongY:
			return D_MATH::Quaternion(D_MATH::Vector3::Forward, DirectX::XM_PIDIV2);
		case Darius::Physics::CapsuleColliderComponent::CapsuleColliderOrientation::AlongZ:
			return D_MATH::Quaternion(D_MATH::Vector3::Up, DirectX::XM_PIDIV2);
		default:
			D_ASSERT(false);
			return D_MATH::Quaternion();
		}
	}
}
