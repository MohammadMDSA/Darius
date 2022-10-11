#include "Physics/pch.hpp"
#include "BoxColliderComponent.hpp"

#include <Debug/DebugDraw.hpp>

namespace Darius::Physics
{

	D_H_COMP_DEF(BoxColliderComponent);

	BoxColliderComponent::BoxColliderComponent() :
		ColliderComponent() {}

	BoxColliderComponent::BoxColliderComponent(D_CORE::Uuid uuid) : \
		ColliderComponent(uuid) {}

	void BoxColliderComponent::Start()
	{
		auto scale = GetTransform().Scale;
		mGeometry = physx::PxBoxGeometry(scale.GetX() / 2.f, scale.GetY() / 2.f, scale.GetZ() / 2.f);

		ColliderComponent::Start();
	}

	void BoxColliderComponent::Update(float dt)
	{
		ColliderComponent::Update(dt);

	}

	void BoxColliderComponent::PreUpdate()
	{
		ColliderComponent::PreUpdate();

		auto transform = GetTransform();
		D_DEBUG_DRAW::DrawCube(transform.Translation, transform.Rotation, transform.Scale, { 0.f, 1.f, 0.f, 1.f });
	}

	physx::PxGeometry* BoxColliderComponent::UpdateAndGetPhysicsGeometry(bool& changed)
	{
		auto scale = GetTransform().Scale;
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

	physx::PxGeometry const* BoxColliderComponent::GetPhysicsGeometry() const
	{
		return &mGeometry;
	}
}
