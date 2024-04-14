#include <pch.hpp>
#include "LaserShoot.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Debug/DebugDraw.hpp>
#include <Physics/PhysicsManager.hpp>
#include <Physics/Components/SphereColliderComponent.hpp>

#ifdef _D_EDITOR
#include <imgui.h>
#endif

#include "LaserShoot.sgenerated.hpp"

using namespace D_MATH;
using namespace D_PHYSICS;

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(LaserShoot);

	LaserShoot::LaserShoot() :
		D_ECS_COMP::BehaviourComponent(),
		mCastType(0)
	{ }

	LaserShoot::LaserShoot(D_CORE::Uuid const& uuid) :
		D_ECS_COMP::BehaviourComponent(uuid),
		mCastType(0)
	{ }

	void LaserShoot::Update(float deltaTime)
	{
		auto trans = GetTransform();
		Vector3 scale = trans->GetLocalScale();

		physx::PxRaycastBuffer rayHit;
		physx::PxSweepBuffer sweepHit;

		static const float maxDist = 30.f;
		static const float timeToDisplay = 1.f;

		switch (mCastType)
		{
		case 0: // Ray
			D_PHYSICS::GetScene()->CastRay_DebugDraw(trans->GetPosition(), trans->GetForward(), maxDist, timeToDisplay, rayHit);
			break;

		case 1:
			D_PHYSICS::GetScene()->CastCapsule_DebugDraw(trans->GetPosition(), trans->GetForward(), maxDist, D_MATH::Max(0.01f, D_MATH::Max(D_MATH::Abs(scale.GetX()), D_MATH::Abs(scale.GetZ()))), D_MATH::Max(0.01f, D_MATH::Abs(scale.GetY())), Quaternion::Identity, timeToDisplay, sweepHit);
			break;

		case 2:
			D_PHYSICS::GetScene()->CastSphere_DebugDraw(trans->GetPosition(), trans->GetForward(), D_MATH::Max(0.01f, D_MATH::Max(D_MATH::Abs(scale.GetX()), D_MATH::Max(D_MATH::Abs(scale.GetZ()), D_MATH::Abs(scale.GetY())))), maxDist, timeToDisplay, sweepHit);
			break;

		case 3:
			D_PHYSICS::GetScene()->CastBox_DebugDraw(trans->GetPosition(), trans->GetForward(), D_MATH::Max(D_MATH::Abs(scale), Vector3(0.01f)) / 2, Quaternion::Identity, maxDist, timeToDisplay, sweepHit);
			break;
		default:
			break;
		}

	}

#ifdef _D_EDITOR
	bool LaserShoot::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();


		// sample field
		D_H_DETAILS_DRAW_PROPERTY("Cast Type");

		valueChanged |= ImGui::Combo("##CastType", &mCastType, "Ray\0Capsule\0Sphere\0Box\0\0");

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

}
