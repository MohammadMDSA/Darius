#include "Physics/pch.hpp"
#include "BoxColliderComponent.hpp"

#include <Debug/DebugDraw.hpp>

#if _D_EDITOR
#include <imgui.h>
#endif

#include "BoxColliderComponent.sgenerated.hpp"

namespace Darius::Physics
{

	D_H_COMP_DEF(BoxColliderComponent);

	BoxColliderComponent::BoxColliderComponent() :
		ColliderComponent(),
		mHalfExtents(0.5f)
	{ }

	BoxColliderComponent::BoxColliderComponent(D_CORE::Uuid uuid) :
		ColliderComponent(uuid),
		mHalfExtents(0.5f)
	{ }

	void BoxColliderComponent::Awake()
	{
		CalculateScaledParameters();
		CalculateGeometry(mGeometry);

		ColliderComponent::Awake();
	}

#ifdef _D_EDITOR
	bool BoxColliderComponent::DrawDetails(float params[])
	{
		bool valueChanged = false;

		valueChanged |= ColliderComponent::DrawDetails(params);

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Half extents
		{
			D_H_DETAILS_DRAW_PROPERTY("Half Extents");
			auto halfExt = GetHalfExtents();
			float drawParams[] = D_H_DRAW_DETAILS_MAKE_VEC_PARAM(0.5f, false);
			if (D_MATH::DrawDetails(halfExt, drawParams))
			{
				SetHalfExtents(halfExt);
				valueChanged = true;
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

	void BoxColliderComponent::CalculateGeometry(physx::PxGeometry& geom) const
	{
		physx::PxBoxGeometry& box = reinterpret_cast<physx::PxBoxGeometry&>(geom);
		box = physx::PxBoxGeometry(D_PHYSICS::GetVec3(mScaledHalfExtents));
	}

	void BoxColliderComponent::OnGizmo() const
	{
		if (!IsActive())
			return;
		auto transform = GetTransform();
		D_DEBUG_DRAW::DrawCube(transform->GetPosition(), transform->GetRotation(), mScaledHalfExtents * 2, 0, { 0.f, 1.f, 0.f, 1.f });
	}

	void BoxColliderComponent::CalculateScaledParameters()
	{
		auto absHalfExt = D_MATH::Abs(GetHalfExtents());
		mScaledHalfExtents = absHalfExt * GetTransform()->GetScale();
		Super::CalculateScaledParameters();
	}

	physx::PxGeometry* BoxColliderComponent::UpdateAndGetPhysicsGeometry(bool& changed)
	{
		auto scale = GetTransform()->GetScale();
		if (!IsDirty() && GetUsedScale().NearEquals(scale, COLLIDER_SCALE_TOLERANCE))
		{
			changed = false;
			return &mGeometry;
		}
		changed = true;

		CalculateScaledParameters();
		CalculateGeometry(mGeometry);

		SetClean();

		return &mGeometry;

	}

	void BoxColliderComponent::SetHalfExtents(D_MATH::Vector3 const& halfExtents)
	{
		auto abs = D_MATH::Abs(halfExtents);

		if (mHalfExtents == abs)
			return;

		mHalfExtents = abs;
		SetDirty();
		
		mChangeSignal(this);
	}
}
