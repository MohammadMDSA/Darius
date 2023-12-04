#include <pch.hpp>
#include "CollisionTest.hpp"

#include <Physics/Components/SphereColliderComponent.hpp>
#include <Physics/Components/CapsuleColliderComponent.hpp>
#include <Physics/Components/BoxColliderComponent.hpp>
#include <Physics/Components/MeshColliderComponent.hpp>
#include <ResourceManager/Resource.hpp>

#ifdef _D_EDITOR
#include <Scene/Utils/GameObjectDragDropPayload.hpp>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "CollisionTest.sgenerated.hpp"

using namespace D_PHYSICS;

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(CollisionTest);

	CollisionTest::CollisionTest() :
		D_ECS_COMP::BehaviourComponent()
	{ }

	CollisionTest::CollisionTest(D_CORE::Uuid uuid) :
		D_ECS_COMP::BehaviourComponent(uuid)
	{ }

	void CollisionTest::Start()
	{
		auto box = GetGameObject()->GetComponent<BoxColliderComponent>();
		auto capsule = GetGameObject()->GetComponent<CapsuleColliderComponent>();
		auto sphere = GetGameObject()->GetComponent<SphereColliderComponent>();
		auto mesh = GetGameObject()->GetComponent<MeshColliderComponent>();

		if (box)
		{
			box->OnColliderContactEnter.ConnectComponent(this, &CollisionTest::OnTouchEnter);
			box->OnColliderContactStay.ConnectComponent(this, &CollisionTest::OnTouchStay);
			box->OnColliderContactLost.ConnectComponent(this, &CollisionTest::OnTouchExit);
		}

		if (capsule)
		{
			capsule->OnColliderContactEnter.ConnectComponent(this, &CollisionTest::OnTouchEnter);
			capsule->OnColliderContactStay.ConnectComponent(this, &CollisionTest::OnTouchStay);
			capsule->OnColliderContactLost.ConnectComponent(this, &CollisionTest::OnTouchExit);
		}

		if (sphere)
		{
			sphere->OnColliderContactEnter.ConnectComponent(this, &CollisionTest::OnTouchEnter);
			sphere->OnColliderContactStay.ConnectComponent(this, &CollisionTest::OnTouchStay);
			sphere->OnColliderContactLost.ConnectComponent(this, &CollisionTest::OnTouchExit);
		}

		if (mesh)
		{
			mesh->OnColliderContactEnter.ConnectComponent(this, &CollisionTest::OnTouchEnter);
			mesh->OnColliderContactStay.ConnectComponent(this, &CollisionTest::OnTouchStay);
			mesh->OnColliderContactLost.ConnectComponent(this, &CollisionTest::OnTouchExit);
		}
	}

	void CollisionTest::Update(float deltaTime)
	{
		
	}

	void CollisionTest::OnTouchEnter(Darius::Physics::ColliderComponent* thisCollider, Darius::Physics::ColliderComponent* otherCollider, D_SCENE::GameObject* otherGameObject, Darius::Physics::HitResult const& Hit)
	{
		if (IsActive())
			D_LOG_DEBUG("Touch Enter " << otherGameObject->GetName());
	}

	void CollisionTest::OnTouchStay(Darius::Physics::ColliderComponent* thisCollider, Darius::Physics::ColliderComponent* otherCollider, D_SCENE::GameObject* otherGameObject, Darius::Physics::HitResult const& Hit)
	{
		if (IsActive())
			D_LOG_DEBUG("Touch Stay " << otherGameObject->GetName());
	}

	void CollisionTest::OnTouchExit(Darius::Physics::ColliderComponent* thisCollider, Darius::Physics::ColliderComponent* otherCollider, D_SCENE::GameObject* otherGameObject, Darius::Physics::HitResult const& Hit)
	{
		if (IsActive())
			D_LOG_DEBUG("Touch Exit " << otherGameObject->GetName());
	}

#ifdef _D_EDITOR
	bool CollisionTest::DrawDetails(float params[])
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
