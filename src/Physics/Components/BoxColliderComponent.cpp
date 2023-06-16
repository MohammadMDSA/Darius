#include "Physics/pch.hpp"
#include "BoxColliderComponent.hpp"

#include <Debug/DebugDraw.hpp>

#include "BoxColliderComponent.sgenerated.hpp"

namespace Darius::Physics
{

	D_H_COMP_DEF(BoxColliderComponent);

	BoxColliderComponent::BoxColliderComponent() :
		ColliderComponent() {}

	BoxColliderComponent::BoxColliderComponent(D_CORE::Uuid uuid) : \
		ColliderComponent(uuid) {}

	void BoxColliderComponent::Awake()
	{
		auto scale = GetTransform()->GetScale();
		mGeometry = physx::PxBoxGeometry(scale.GetX() / 2.f, scale.GetY() / 2.f, scale.GetZ() / 2.f);

		ColliderComponent::Awake();
	}

	bool BoxColliderComponent::DrawDetails(float params[])
	{
		bool valueChanged = false;

		valueChanged |= ColliderComponent::DrawDetails(params);

		return valueChanged;
	}

	void BoxColliderComponent::OnGizmo() const
	{
		auto transform = GetTransform();
		D_DEBUG_DRAW::DrawCube(transform->GetPosition(), transform->GetRotation(), transform->GetScale(), 0, { 0.f, 1.f, 0.f, 1.f });
	}

	physx::PxGeometry* BoxColliderComponent::UpdateAndGetPhysicsGeometry(bool& changed)
	{
		auto scale = GetTransform()->GetScale();
		auto size = physx::PxVec3(scale.GetX() / 2.f, scale.GetY() / 2.f, scale.GetZ() / 2.f);
		if (mGeometry.halfExtents == size)
		{
			changed = false;
			return &mGeometry;
		}

		mGeometry = physx::PxBoxGeometry(size);
		changed = true;
		return &mGeometry;

	}
}
