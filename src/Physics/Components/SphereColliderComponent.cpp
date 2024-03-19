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
		auto rot = transform->GetRotation();
		auto offset = rot * GetScaledCenterOffset();
		D_DEBUG_DRAW::DrawSphere(transform->GetPosition() + offset, GetScaledRadius() + 0.03f, 0, { 0.f, 1.f, 0.f, 1.f });
	}
#endif

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

	bool SphereColliderComponent::CalculateGeometry(physx::PxGeometry& geom) const
	{
		physx::PxSphereGeometry& sphere = reinterpret_cast<physx::PxSphereGeometry&>(geom);
		sphere = physx::PxSphereGeometry(mScaledRadius);

		return true;
	}
}
