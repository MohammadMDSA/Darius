#include "Physics/pch.hpp"
#include "RigidbodyComponent.hpp"

#include "Physics/PhysicsManager.hpp"
#include "Physics/PhysicsScene.hpp"

#include <imgui.h>

using namespace physx;
using namespace D_MATH;

namespace Darius::Physics
{
	D_H_COMP_DEF(RigidbodyComponent);

	RigidbodyComponent::RigidbodyComponent() :
		ComponentBase(),
		mKinematic(false) {}

	RigidbodyComponent::RigidbodyComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid),
		mKinematic(false) {}

	void RigidbodyComponent::Start()
	{
		mActor = D_PHYSICS::PhysicsScene::AddDynamicActor(GetGameObject(), false);

		SetKinematic(mKinematic);
	}

	void RigidbodyComponent::Serialize(D_SERIALIZATION::Json& json) const
	{
		json["Kinematic"] = IsKinematic();
	}

	void RigidbodyComponent::Deserialize(D_SERIALIZATION::Json const& json)
	{
		D_H_DESERIALIZE(Kinematic);

		if (mActor)
		{
			SetKinematic(mKinematic);
		}
	}

	void RigidbodyComponent::OnDestroy()
	{
		D_PHYSICS::PhysicsScene::RemoveDynamicActor(GetGameObject());
	}

	void RigidbodyComponent::Update(float)
	{
		auto preTrans = GetTransform();
		D_MATH::Transform physicsTrans = D_PHYSICS::GetTransform(mActor->getGlobalPose());
		preTrans.Translation = physicsTrans.Translation;
		preTrans.Rotation = physicsTrans.Rotation;
		SetTransform(preTrans);
	}

	void RigidbodyComponent::PreUpdate()
	{
		mActor->setGlobalPose(D_PHYSICS::GetTransform(GetTransform()));
	}

	void RigidbodyComponent::SetKinematic(bool value)
	{
		mActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, value);
	}

	bool RigidbodyComponent::IsKinematic() const
	{
		return mActor->getRigidBodyFlags().isSet(PxRigidBodyFlag::eKINEMATIC);
	}

	Vector3 RigidbodyComponent::GetLinearVelocity() const
	{
		auto v = mActor->getLinearVelocity();
		return Vector3(reinterpret_cast<float*>(&v));
	}

	void RigidbodyComponent::SetLinearVelocity(Vector3 const& v, bool autoWake)
	{
		mActor->setLinearVelocity(PxVec3(v.GetX(), v.GetY(), v.GetZ()));
	}

#ifdef _D_EDITOR
	bool RigidbodyComponent::DrawDetails(float[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Kinematic property
		{
			D_H_DETAILS_DRAW_PROPERTY("Kinematic");
			bool kinematic = IsKinematic();
			if (ImGui::Checkbox("##kinematic", &kinematic))
			{
				SetKinematic(kinematic);
				valueChanged = true;
			}

		}

		ImGui::NewLine();
		ImGui::NewLine();


		ImGui::BeginDisabled();
		// Velocity
		{
			float params[] = D_H_DRAW_DETAILS_MAKE_VEC_PARAM(0.f, false);
			auto v = GetLinearVelocity();
			D_H_DETAILS_DRAW_PROPERTY("Linear Velocity");
			D_MATH::DrawDetails(v, params);
		}
		ImGui::EndDisabled();

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

}
