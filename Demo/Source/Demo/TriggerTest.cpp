#include <pch.hpp>
#include "TriggerTest.hpp"

#include <Physics/Components/SphereColliderComponent.hpp>
#include <Physics/Components/CapsuleColliderComponent.hpp>
#include <physics/Components/BoxColliderComponent.hpp>
#include <Physics/Components/MeshColliderComponent.hpp>
#include <ResourceManager/Resource.hpp>

#ifdef _D_EDITOR
#include <Scene/Utils/GameObjectDragDropPayload.hpp>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "TriggerTest.sgenerated.hpp"

using namespace D_PHYSICS;

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(TriggerTest);

	TriggerTest::TriggerTest() :
		D_ECS_COMP::BehaviourComponent()
	{ }

	TriggerTest::TriggerTest(D_CORE::Uuid const& uuid) :
		D_ECS_COMP::BehaviourComponent(uuid)
	{ }

	void TriggerTest::Start()
	{
		auto box = GetGameObject()->GetComponent<BoxColliderComponent>();
		auto capsule = GetGameObject()->GetComponent<CapsuleColliderComponent>();
		auto sphere = GetGameObject()->GetComponent<SphereColliderComponent>();
		auto mesh = GetGameObject()->GetComponent<MeshColliderComponent>();

		if (box)
		{
			box->OnTriggerEnter.ConnectComponent(this, &TriggerTest::OnTriggerEnter);
			box->OnTriggerExit.ConnectComponent(this, &TriggerTest::OnTriggerExit);
		}

		if (capsule)
		{
			capsule->OnTriggerEnter.ConnectComponent(this, &TriggerTest::OnTriggerEnter);
			capsule->OnTriggerExit.ConnectComponent(this, &TriggerTest::OnTriggerExit);
		}

		if (sphere)
		{
			sphere->OnTriggerEnter.ConnectComponent(this, &TriggerTest::OnTriggerEnter);
			sphere->OnTriggerExit.ConnectComponent(this, &TriggerTest::OnTriggerExit);
		}

		if (mesh)
		{
			mesh->OnTriggerEnter.ConnectComponent(this, &TriggerTest::OnTriggerEnter);
			mesh->OnTriggerExit.ConnectComponent(this, &TriggerTest::OnTriggerExit);
		}

	}

	void TriggerTest::Update(float deltaTime)
	{

	}

	void TriggerTest::OnTriggerEnter(Darius::Physics::ColliderComponent* thisCollider, D_SCENE::GameObject* otherGameObject)
	{
		if (IsActive())
			D_LOG_DEBUG("Trigger Enter " << otherGameObject->GetName());
	}

	void TriggerTest::OnTriggerExit(Darius::Physics::ColliderComponent* thisCollider, D_SCENE::GameObject* otherGameObject)
	{
		if (IsActive())
			D_LOG_DEBUG("Trigger Exit " << otherGameObject->GetName());
	}


#ifdef _D_EDITOR
	bool TriggerTest::DrawDetails(float params[])
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
