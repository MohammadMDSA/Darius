#include "Physics/pch.hpp"
#include "SphereColliderComponent.hpp"

#include <Debug/DebugDraw.hpp>

#include "SphereColliderComponent.sgenerated.hpp"

namespace Darius::Physics
{

	D_H_COMP_DEF(SphereColliderComponent);

	SphereColliderComponent::SphereColliderComponent() :
		ColliderComponent() {}

	SphereColliderComponent::SphereColliderComponent(D_CORE::Uuid uuid) : \
		ColliderComponent(uuid) {}

	void SphereColliderComponent::Awake()
	{
		mGeometry = physx::PxSphereGeometry(GetRadius());

		ColliderComponent::Awake();
	}

	bool SphereColliderComponent::DrawDetails(float params[])
	{
		bool valueChanged = false;

		valueChanged |= ColliderComponent::DrawDetails(params);

		return valueChanged;
	}

	void SphereColliderComponent::OnGizmo() const
	{
		auto transform = GetTransform();
		D_DEBUG_DRAW::DrawSphere(transform->GetPosition(), GetRadius() + 0.03f, 0, { 0.f, 1.f, 0.f, 1.f });
	}

	physx::PxGeometry* SphereColliderComponent::UpdateAndGetPhysicsGeometry(bool& changed)
	{
		auto radius = GetRadius();
		if (mGeometry.radius == radius)
		{
			changed = false;
			return &mGeometry;
		}

		mGeometry = physx::PxSphereGeometry(radius);
		changed = true;
		return &mGeometry;

	}

	physx::PxGeometry const* SphereColliderComponent::GetPhysicsGeometry() const
	{
		return &mGeometry;
	}
}
