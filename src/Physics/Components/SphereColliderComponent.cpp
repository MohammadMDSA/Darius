#include "Physics/pch.hpp"
#include "SphereColliderComponent.hpp"

#include <Debug/DebugDraw.hpp>

#if _D_EDITOR
#include <imgui.h>
#endif // _D_EDITOR


#include "SphereColliderComponent.sgenerated.hpp"

namespace Darius::Physics
{

	D_H_COMP_DEF(SphereColliderComponent);

	SphereColliderComponent::SphereColliderComponent() :
		ColliderComponent(),
		mRadius(0.5),
		mScaledRadius(0.f)
	{
		SetDirty();
	}

	SphereColliderComponent::SphereColliderComponent(D_CORE::Uuid uuid) :
		ColliderComponent(uuid),
		mRadius(0.5),
		mScaledRadius(0.f)
	{
		SetDirty();
	}

	void SphereColliderComponent::Awake()
	{
		CalculateScaledParameters();
		CalculateGeometry(mGeometry);

		ColliderComponent::Awake();
	}

#ifdef _D_EDITOR
	bool SphereColliderComponent::DrawDetails(float params[])
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

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}

	void SphereColliderComponent::OnGizmo() const
	{
		if (!IsActive())
			return;

		auto transform = GetTransform();
		D_DEBUG_DRAW::DrawSphere(transform->GetPosition(), GetScaledRadius() + 0.03f, 0, { 0.f, 1.f, 0.f, 1.f });
	}
#endif

	physx::PxGeometry* SphereColliderComponent::UpdateAndGetPhysicsGeometry(bool& changed)
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

	void SphereColliderComponent::SetRadius(float radius)
	{
		if (mRadius == radius)
			return;

		mRadius = radius;
		SetDirty();

		mChangeSignal(this);
	}

	void SphereColliderComponent::CalculateScaledParameters()
	{
		auto scale = GetTransform()->GetScale();
		auto radiusScale = D_MATH::GetMaxComponent(D_MATH::Abs(scale));

		mScaledRadius = D_MATH::Max(radiusScale * GetRadius(), MinRadius);

		Super::CalculateScaledParameters();
	}

	void SphereColliderComponent::CalculateGeometry(physx::PxGeometry& geom) const
	{
		physx::PxSphereGeometry& sphere = reinterpret_cast<physx::PxSphereGeometry&>(geom);
		sphere = physx::PxSphereGeometry(mScaledRadius);
	}
}
