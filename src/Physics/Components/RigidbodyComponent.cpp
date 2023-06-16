#include "Physics/pch.hpp"
#include "RigidbodyComponent.hpp"

#include "Physics/PhysicsManager.hpp"
#include "Physics/PhysicsScene.hpp"

#ifdef _D_EDITOR
#include <imgui.h>
#include <imgui_internal.h>
#endif

#include "RigidbodyComponent.sgenerated.hpp"

#define VEC3_2_PX(v) PxVec3(v.GetX(), v.GetY(), v.GetZ())

using namespace physx;
using namespace D_MATH;

namespace Darius::Physics
{
	D_H_COMP_DEF(RigidbodyComponent);

	RigidbodyComponent::RigidbodyComponent() :
		ComponentBase(),
		mKinematic(false),
		mUsingGravity(true)
	{
		ZeroMemory(mRotationConstraints, 3 * sizeof(bool));
		ZeroMemory(mPositionConstraints, 3 * sizeof(bool));
	}

	RigidbodyComponent::RigidbodyComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid),
		mKinematic(false),
		mUsingGravity(true)
	{
		ZeroMemory(mRotationConstraints, 3 * sizeof(bool));
		ZeroMemory(mPositionConstraints, 3 * sizeof(bool));
	}

	void RigidbodyComponent::Awake()
	{
		mActor = D_PHYSICS::PhysicsScene::AddDynamicActor(GetGameObject(), false);
		D_ASSERT(mActor);

		SetKinematic(mKinematic);
		SetUsingGravity(mUsingGravity);
		SetRotationConstraintsX(mRotationConstraints[0]);
		SetRotationConstraintsY(mRotationConstraints[1]);
		SetRotationConstraintsZ(mRotationConstraints[2]);
		SetPositionConstraintsX(mPositionConstraints[0]);
		SetPositionConstraintsY(mPositionConstraints[1]);
		SetPositionConstraintsZ(mPositionConstraints[2]);
	}

	void RigidbodyComponent::OnDestroy()
	{
		D_PHYSICS::PhysicsScene::RemoveDynamicActor(GetGameObject());
	}

	void RigidbodyComponent::Update(float)
	{
		auto preTrans = GetTransform();
		D_MATH::Transform physicsTrans = D_PHYSICS::GetTransform(mActor->getGlobalPose());

		physicsTrans.Scale = preTrans->GetScale();
		GetTransform()->SetWorld(Matrix4(physicsTrans.GetWorld()));
	}

	void RigidbodyComponent::PreUpdate()
	{
		mActor->setGlobalPose(D_PHYSICS::GetTransform(GetTransform()->GetTransformData()));
	}

	bool RigidbodyComponent::IsUsingGravity() const
	{
		if (!mActor)
			return mUsingGravity;

		auto flags = mActor->getActorFlags();

		return !flags.isSet(PxActorFlag::eDISABLE_GRAVITY);
	}

	void RigidbodyComponent::SetUsingGravity(bool enable)
	{
		if (mActor)
			mActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !enable);

		mUsingGravity = enable;
		mChangeSignal();
	}

	void RigidbodyComponent::SetKinematic(bool value)
	{
		if (mActor)
			mActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, value);
		mKinematic = value;
		mChangeSignal();
	}

	bool RigidbodyComponent::IsKinematic() const
	{
		if (!mActor)
			return mKinematic;

		return mActor->getRigidBodyFlags().isSet(PxRigidBodyFlag::eKINEMATIC);
	}

	Vector3 RigidbodyComponent::GetLinearVelocity() const
	{
		if (!mActor)
			return Vector3(kZero);

		auto v = mActor->getLinearVelocity();
		return Vector3(reinterpret_cast<float*>(&v));
	}

	void RigidbodyComponent::SetLinearVelocity(Vector3 const& v, bool autoWake)
	{
		if (!mActor)
			return;

		mActor->setLinearVelocity(VEC3_2_PX(v));
		mChangeSignal();
	}

	Vector3 RigidbodyComponent::GetAngularVelocity() const
	{
		if (!mActor)
			return Vector3(kZero);

		auto v = mActor->getAngularVelocity();
		return Vector3(reinterpret_cast<float*>(&v));
	}

	bool RigidbodyComponent::GetRotationConstraintsX() const
	{
		if (!mActor)
			return mRotationConstraints[0];

		auto flags = mActor->getRigidDynamicLockFlags();
		return flags.isSet(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X);
	}

	bool RigidbodyComponent::GetRotationConstraintsY() const
	{
		if (!mActor)
			return mRotationConstraints[1];

		auto flags = mActor->getRigidDynamicLockFlags();
		return flags.isSet(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y);
	}

	bool RigidbodyComponent::GetRotationConstraintsZ() const
	{
		if (!mActor)
			return mRotationConstraints[2];

		auto flags = mActor->getRigidDynamicLockFlags();
		return flags.isSet(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z);
	}

	void RigidbodyComponent::SetRotationConstraintsX(bool enable)
	{
		if (mActor)
			mActor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, enable);

		mRotationConstraints[0] = enable;
		mChangeSignal();
	}

	void RigidbodyComponent::SetRotationConstraintsY(bool enable)
	{
		if (mActor)
			mActor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, enable);

		mRotationConstraints[1] = enable;
		mChangeSignal();
	}

	void RigidbodyComponent::SetRotationConstraintsZ(bool enable)
	{
		if (mActor)
			mActor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, enable);

		mRotationConstraints[2] = enable;
		mChangeSignal();
	}

	bool RigidbodyComponent::GetPositionConstraintsX() const
	{
		if (!mActor)
			return mPositionConstraints[0];

		auto flags = mActor->getRigidDynamicLockFlags();
		return flags.isSet(PxRigidDynamicLockFlag::eLOCK_LINEAR_X);
	}

	bool RigidbodyComponent::GetPositionConstraintsY() const
	{
		if (!mActor)
			return mPositionConstraints[1];

		auto flags = mActor->getRigidDynamicLockFlags();
		return flags.isSet(PxRigidDynamicLockFlag::eLOCK_LINEAR_Y);
	}

	bool RigidbodyComponent::GetPositionConstraintsZ() const
	{
		if (!mActor)
			return mPositionConstraints[2];

		auto flags = mActor->getRigidDynamicLockFlags();
		return flags.isSet(PxRigidDynamicLockFlag::eLOCK_LINEAR_Z);
	}

	void RigidbodyComponent::SetPositionConstraintsX(bool enable)
	{
		if (mActor)
			mActor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_X, enable);

		mPositionConstraints[0] = enable;
		mChangeSignal();
	}

	void RigidbodyComponent::SetPositionConstraintsY(bool enable)
	{
		if (mActor)
			mActor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, enable);

		mPositionConstraints[1] = enable;
		mChangeSignal();
	}

	void RigidbodyComponent::SetPositionConstraintsZ(bool enable)
	{
		if (mActor)
			mActor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, enable);

		mPositionConstraints[2] = enable;
		mChangeSignal();
	}

	void RigidbodyComponent::SetAngularVelocity(Vector3 const& v, bool autoWake)
	{
		if (!mActor)
			return;

		mActor->setAngularVelocity(VEC3_2_PX(v));
		mChangeSignal();
	}

	void RigidbodyComponent::AddForce(D_MATH::Vector3 const& f)
	{
		mActor->addForce(VEC3_2_PX(f));
		mChangeSignal();
	}

	void RigidbodyComponent::ClearForce()
	{
		mActor->clearForce();
		mChangeSignal();
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

		// Gravity property
		{
			D_H_DETAILS_DRAW_PROPERTY("Use Gravity");
			bool gravity = IsUsingGravity();
			if (ImGui::Checkbox("##gravity", &gravity))
			{
				SetUsingGravity(gravity);
				valueChanged = true;
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		ImGui::Separator();
		if (ImGui::TreeNode("Constraints"))
		{
			D_H_DETAILS_DRAW_BEGIN_TABLE("Constraints");

			// Position Constraints
			{
				D_H_DETAILS_DRAW_PROPERTY("Freese Position");
				valueChanged |= DrawPositionConstraints();
			}

			// Rotation Constraints
			{
				D_H_DETAILS_DRAW_PROPERTY("Freese Rotation");
				valueChanged |= DrawRotationConstraints();
			}


			D_H_DETAILS_DRAW_END_TABLE();

			ImGui::TreePop();
		}

		ImGui::Separator();

		if (ImGui::TreeNode("Details"))
		{
			ImGui::BeginDisabled();

			D_H_DETAILS_DRAW_BEGIN_TABLE("Details");

			float simpleVecParams[] = D_H_DRAW_DETAILS_MAKE_VEC_PARAM(0.f, false);

			// Linear Velocity
			{
				auto v = GetLinearVelocity();
				D_H_DETAILS_DRAW_PROPERTY("Linear Velocity");
				D_MATH::DrawDetails(v, simpleVecParams);
			}

			// Angular Velocity
			{
				auto v = GetAngularVelocity();
				D_H_DETAILS_DRAW_PROPERTY("Angular Velocity");
				D_MATH::DrawDetails(v, simpleVecParams);
			}

			ImGui::EndDisabled();
			D_H_DETAILS_DRAW_END_TABLE();

			ImGui::TreePop();
		}

		return valueChanged;
	}

	bool RigidbodyComponent::DrawRotationConstraints()
	{
		auto valueChanged = false;

		ImGui::PushID("RotationConstraints");


		ImGui::BeginGroup();
		bool lockX = GetRotationConstraintsX();
		if (ImGui::Checkbox("X", &lockX))
		{
			valueChanged = true;
			SetRotationConstraintsX(lockX);
		}
		ImGui::EndGroup();

		ImGui::SameLine(100);

		ImGui::BeginGroup();
		bool lockY = GetRotationConstraintsY();
		if (ImGui::Checkbox("Y", &lockY))
		{
			valueChanged = true;
			SetRotationConstraintsY(lockY);
		}
		ImGui::EndGroup();

		ImGui::SameLine(150);

		ImGui::BeginGroup();
		bool lockZ = GetRotationConstraintsZ();
		if (ImGui::Checkbox("Z", &lockZ))
		{
			valueChanged = true;
			SetRotationConstraintsZ(lockZ);
		}
		ImGui::EndGroup();

		ImGui::PopID();

		return valueChanged;
	}

	bool RigidbodyComponent::DrawPositionConstraints()
	{
		auto valueChanged = false;

		ImGui::PushID("PositionConstraints");


		ImGui::BeginGroup();
		bool lockX = GetPositionConstraintsX();
		if (ImGui::Checkbox("X", &lockX))
		{
			valueChanged = true;
			SetPositionConstraintsX(lockX);
		}
		ImGui::EndGroup();

		ImGui::SameLine(100);

		ImGui::BeginGroup();
		bool lockY = GetPositionConstraintsY();
		if (ImGui::Checkbox("Y", &lockY))
		{
			valueChanged = true;
			SetPositionConstraintsY(lockY);
		}
		ImGui::EndGroup();

		ImGui::SameLine(150);

		ImGui::BeginGroup();
		bool lockZ = GetPositionConstraintsZ();
		if (ImGui::Checkbox("Z", &lockZ))
		{
			valueChanged = true;
			SetPositionConstraintsZ(lockZ);
		}
		ImGui::EndGroup();

		ImGui::PopID();

		return valueChanged;
	}
#endif

}
