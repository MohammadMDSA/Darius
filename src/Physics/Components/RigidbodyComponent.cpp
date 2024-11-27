#include "Physics/pch.hpp"
#include "RigidbodyComponent.hpp"

#include "Physics/PhysicsManager.hpp"
#include "Physics/PhysicsScene.hpp"
#include "Physics/Components/CapsuleColliderComponent.hpp"

#include <Renderer/Components/MeshRendererComponent.hpp>

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

	PxForceMode::Enum ToNativeForceMode(ForceMode mode)
	{
		switch (mode)
		{
		case Darius::Physics::ForceMode::Force:
			return PxForceMode::eFORCE;
		case Darius::Physics::ForceMode::Impulse:
			return PxForceMode::eIMPULSE;
		case Darius::Physics::ForceMode::VelocityChange:
			return PxForceMode::eVELOCITY_CHANGE;
		case Darius::Physics::ForceMode::Acceleration:
			return PxForceMode::eACCELERATION;
		default:
			D_ASSERT_NOENTRY();
			return PxForceMode::eFORCE;
		}
	}

	RigidbodyComponent::RigidbodyComponent() :
		ComponentBase(),
		mKinematic(false),
		mUsingGravity(true),
		mCenterOfMass(Vector3::Zero),
		mMass(1.f)
	{
		mRotationConstraintsX = false;
		mRotationConstraintsY = false;
		mRotationConstraintsZ = false;
		mPositionConstraintsX = false;
		mPositionConstraintsY = false;
		mPositionConstraintsZ = false;
	}

	RigidbodyComponent::RigidbodyComponent(D_CORE::Uuid const& uuid) :
		ComponentBase(uuid),
		mKinematic(false),
		mUsingGravity(true),
		mCenterOfMass(Vector3::Zero),
		mMass(1.f)
	{
		mRotationConstraintsX = false;
		mRotationConstraintsY = false;
		mRotationConstraintsZ = false;
		mPositionConstraintsX = false;
		mPositionConstraintsY = false;
		mPositionConstraintsZ = false;
	}

	void RigidbodyComponent::Awake()
	{
		mActor = D_PHYSICS::GetScene()->FindOrCreatePhysicsActor(GetGameObject());
		D_ASSERT(mActor.IsValid());
		mActor->SetDynamic(true);
	}

	void RigidbodyComponent::Start()
	{
		mActor->InitializeActor();
		D_ASSERT(mActor.IsValid());

		SetDirty();
		SetKinematic(mKinematic);
		SetUsingGravity(mUsingGravity);
		SetRotationConstraintsX(mRotationConstraintsX);
		SetRotationConstraintsY(mRotationConstraintsY);
		SetRotationConstraintsZ(mRotationConstraintsZ);
		SetPositionConstraintsX(mPositionConstraintsX);
		SetPositionConstraintsY(mPositionConstraintsY);
		SetPositionConstraintsZ(mPositionConstraintsZ);
		SetCenterOfMass(mCenterOfMass);
		SetMass(mMass);
		mActor->GetDynamicActor()->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
		SetClean();
	}

	void RigidbodyComponent::OnDestroy()
	{
		if(mActor.IsValid())
		{
			mActor->SetDynamic(false);

		}
	}

	void RigidbodyComponent::OnActivate()
	{
		if(mActor.IsValid())
		{
			mActor->GetDynamicActor()->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
		}
	}

	void RigidbodyComponent::OnDeactivate()
	{
		if(mActor.IsValid())
		{
			mActor->GetDynamicActor()->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, true);
		}
	}

	bool RigidbodyComponent::IsUsingGravity() const
	{
		if(!mActor.IsValid())
			return mUsingGravity;

		auto flags = mActor->GetDynamicActor()->getActorFlags();

		return !flags.isSet(PxActorFlag::eDISABLE_GRAVITY);
	}

	void RigidbodyComponent::SetUsingGravity(bool enable)
	{
		if(mUsingGravity == enable && !IsDirty())
			return;

		if(mActor.IsValid())
			mActor->GetDynamicActor()->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !enable);

		mUsingGravity = enable;
		mChangeSignal(this);
	}

	void RigidbodyComponent::SetKinematic(bool value)
	{
		if(mKinematic == value && !IsDirty())
			return;

		if(mActor.IsValid())
			mActor->GetDynamicActor()->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, value);
		mKinematic = value;
		mChangeSignal(this);
	}

	bool RigidbodyComponent::IsKinematic() const
	{
		return mKinematic;
	}

	Vector3 RigidbodyComponent::GetLinearVelocity() const
	{
		if(!mActor.IsValid())
			return Vector3(kZero);

		auto v = mActor->GetDynamicActor()->getLinearVelocity();
		return Vector3(reinterpret_cast<float*>(&v));
	}

	void RigidbodyComponent::SetLinearVelocity(Vector3 const& v, bool autoWake)
	{
		if(!mActor.IsValid())
			return;

		if(v.Equals(GetLinearVelocity()))
			return;

		mActor->GetDynamicActor()->setLinearVelocity(VEC3_2_PX(v));
		mChangeSignal(this);
	}

	Vector3 RigidbodyComponent::GetAngularVelocity() const
	{
		if(!mActor.IsValid())
			return Vector3::Zero;

		auto v = mActor->GetDynamicActor()->getAngularVelocity();
		return Vector3(reinterpret_cast<float*>(&v));
	}

	void RigidbodyComponent::SetCenterOfMass(Vector3 const& c)
	{

		if(c.Equals(mCenterOfMass) && !IsDirty())
			return;

		if(mActor.IsValid())
			mActor->GetDynamicActor()->setCMassLocalPose(PxTransform(VEC3_2_PX(c)));

		mCenterOfMass = c;

		mChangeSignal(this);
	}

	Vector3 RigidbodyComponent::GetCenterOfMass() const
	{
		return mCenterOfMass;
	}

	void RigidbodyComponent::SetMass(float mass)
	{
		if(mass == mMass && !IsDirty())
			return;

		if(mActor.IsValid())
			mActor->GetDynamicActor()->setMass(mass);

		mMass = mass;

		mChangeSignal(this);
	}

	bool RigidbodyComponent::GetRotationConstraintsX() const
	{
		return mRotationConstraintsX;
	}

	bool RigidbodyComponent::GetRotationConstraintsY() const
	{
		return mRotationConstraintsY;
	}

	bool RigidbodyComponent::GetRotationConstraintsZ() const
	{
		return mRotationConstraintsZ;
	}

	void RigidbodyComponent::SetRotationConstraintsX(bool enable)
	{
		if(mRotationConstraintsX == enable && !IsDirty())
			return;

		if(mActor.IsValid())
			mActor->GetDynamicActor()->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, enable);

		mRotationConstraintsX = enable;
		mChangeSignal(this);
	}

	void RigidbodyComponent::SetRotationConstraintsY(bool enable)
	{
		if(mRotationConstraintsY == enable && !IsDirty())
			return;

		if(mActor.IsValid())
			mActor->GetDynamicActor()->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, enable);

		mRotationConstraintsY = enable;
		mChangeSignal(this);
	}

	void RigidbodyComponent::SetRotationConstraintsZ(bool enable)
	{
		if(mRotationConstraintsZ == enable && !IsDirty())
			return;

		if(mActor.IsValid())
			mActor->GetDynamicActor()->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, enable);

		mRotationConstraintsZ = enable;
		mChangeSignal(this);
	}

	bool RigidbodyComponent::GetPositionConstraintsX() const
	{
		return mPositionConstraintsX;
	}

	bool RigidbodyComponent::GetPositionConstraintsY() const
	{
		return mPositionConstraintsY;
	}

	bool RigidbodyComponent::GetPositionConstraintsZ() const
	{
		return mPositionConstraintsZ;
	}

	void RigidbodyComponent::SetPositionConstraintsX(bool enable)
	{
		if(mPositionConstraintsX == enable && !IsDirty())
			return;

		if(mActor.IsValid())
			mActor->GetDynamicActor()->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_X, enable);

		mPositionConstraintsX = enable;
		mChangeSignal(this);
	}

	void RigidbodyComponent::SetPositionConstraintsY(bool enable)
	{
		if(mPositionConstraintsY == enable && !IsDirty())
			return;

		if(mActor.IsValid())
			mActor->GetDynamicActor()->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, enable);

		mPositionConstraintsY = enable;
		mChangeSignal(this);
	}

	void RigidbodyComponent::SetPositionConstraintsZ(bool enable)
	{
		if(mPositionConstraintsZ == enable && !IsDirty())
			return;

		if(mActor.IsValid())
			mActor->GetDynamicActor()->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, enable);

		mPositionConstraintsZ = enable;
		mChangeSignal(this);
	}

	void RigidbodyComponent::SetAngularVelocity(Vector3 const& v, bool autoWake)
	{
		if(!mActor.IsValid())
			return;

		if(v.Equals(GetAngularVelocity()))
			return;

		mActor->GetDynamicActor()->setAngularVelocity(VEC3_2_PX(v), autoWake);
		mChangeSignal(this);
	}

	void RigidbodyComponent::AddForce(D_MATH::Vector3 const& f, ForceMode mode)
	{
		mActor->GetDynamicActor()->addForce(VEC3_2_PX(f), ToNativeForceMode(mode));
	}

	void RigidbodyComponent::ClearForce()
	{
		mActor->GetDynamicActor()->clearForce();
	}

#ifdef _D_EDITOR

	void RigidbodyComponent::OnPostComponentAddInEditor()
	{
		Super::OnPostComponentAddInEditor();

		bool addedDefault = false;

		if(auto sm = GetGameObject()->GetComponent<D_RENDERER::MeshRendererComponent>())
		{
			auto trans = GetTransform();
			auto localCenter = sm->GetAabb().GetCenter() - trans->GetPosition();
			SetCenterOfMass(trans->GetRotation() * localCenter);
		}
	}

	bool RigidbodyComponent::DrawDetails(float[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Kinematic property
		{
			D_H_DETAILS_DRAW_PROPERTY("Kinematic");
			bool kinematic = IsKinematic();
			if(ImGui::Checkbox("##kinematic", &kinematic))
			{
				SetKinematic(kinematic);
				valueChanged = true;
			}

		}

		// Gravity property
		{
			D_H_DETAILS_DRAW_PROPERTY("Use Gravity");
			bool gravity = IsUsingGravity();
			if(ImGui::Checkbox("##gravity", &gravity))
			{
				SetUsingGravity(gravity);
				valueChanged = true;
			}
		}

		// Center of Mass
		{
			D_H_DETAILS_DRAW_PROPERTY("Center of Mass");
			auto value = GetCenterOfMass();
			if(D_MATH::DrawDetails(value, Vector3::Zero))
			{
				SetCenterOfMass(value);
				valueChanged = true;
			}
		}

		// Mass
		{
			D_H_DETAILS_DRAW_PROPERTY("Mass");
			auto value = GetMass();
			if(ImGui::DragFloat("##Mass", &value))
			{
				SetMass(value);
				valueChanged = true;
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		ImGui::Separator();
		if(ImGui::TreeNode("Constraints"))
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

		if(ImGui::TreeNode("Details"))
		{
			ImGui::BeginDisabled();

			D_H_DETAILS_DRAW_BEGIN_TABLE("Details");

			// Linear Velocity
			{
				auto v = GetLinearVelocity();
				D_H_DETAILS_DRAW_PROPERTY("Linear Velocity");
				D_MATH::DrawDetails(v);
			}

			// Angular Velocity
			{
				auto v = GetAngularVelocity();
				D_H_DETAILS_DRAW_PROPERTY("Angular Velocity");
				D_MATH::DrawDetails(v);
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
		if(ImGui::Checkbox("X", &lockX))
		{
			valueChanged = true;
			SetRotationConstraintsX(lockX);
		}
		ImGui::EndGroup();

		ImGui::SameLine(100);

		ImGui::BeginGroup();
		bool lockY = GetRotationConstraintsY();
		if(ImGui::Checkbox("Y", &lockY))
		{
			valueChanged = true;
			SetRotationConstraintsY(lockY);
		}
		ImGui::EndGroup();

		ImGui::SameLine(150);

		ImGui::BeginGroup();
		bool lockZ = GetRotationConstraintsZ();
		if(ImGui::Checkbox("Z", &lockZ))
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
		if(ImGui::Checkbox("X", &lockX))
		{
			valueChanged = true;
			SetPositionConstraintsX(lockX);
		}
		ImGui::EndGroup();

		ImGui::SameLine(100);

		ImGui::BeginGroup();
		bool lockY = GetPositionConstraintsY();
		if(ImGui::Checkbox("Y", &lockY))
		{
			valueChanged = true;
			SetPositionConstraintsY(lockY);
		}
		ImGui::EndGroup();

		ImGui::SameLine(150);

		ImGui::BeginGroup();
		bool lockZ = GetPositionConstraintsZ();
		if(ImGui::Checkbox("Z", &lockZ))
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
