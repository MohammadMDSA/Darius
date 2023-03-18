#include <pch.hpp>
#include "LaserShoot.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Debug/DebugDraw.hpp>
#include <Physics/PhysicsManager.hpp>

#ifdef _D_EDITOR
#include <imgui.h>
#endif

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(LaserShoot);

	LaserShoot::LaserShoot() :
		D_ECS_COMP::BehaviourComponent()
	{ }

	LaserShoot::LaserShoot(D_CORE::Uuid uuid) :
		D_ECS_COMP::BehaviourComponent(uuid)
	{ }

	void LaserShoot::Start()
	{
		
	}

	void LaserShoot::Update(float deltaTime)
	{
		auto frame = D_TIME::GetFrameCount();

		auto trans = GetTransform();
		auto dir = trans.Rotation.GetForward();

		D_DEBUG_DRAW::DrawSphere(trans.Translation + dir * 3, 0.5, 0, { 0.f, 1.f, 0.f, 1.f });

		physx::PxRaycastBuffer hit;
		if (D_PHYSICS::Raycast(trans.Translation + dir * 2, dir, 100, hit))
			D_DEBUG_DRAW::DrawCube(D_PHYSICS::GetVec3(hit.block.position), D_MATH::LookAt(D_PHYSICS::GetVec3(hit.block.normal), D_MATH::Vector3::Up), D_MATH::Vector3(0.5f, 0.5f, 0.5f), 1, {1.f, 0.f, 0.f, 1.f});

	}

#ifdef _D_EDITOR
	bool LaserShoot::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();


		// sample field
		D_H_DETAILS_DRAW_PROPERTY("field");

		float val;
		valueChanged |= ImGui::InputFloat("##val", &val);

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

}
