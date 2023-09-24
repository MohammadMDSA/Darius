#include <pch.hpp>
#include "Targeter.hpp"

#include <ResourceManager/Resource.hpp>

#ifdef _D_EDITOR
#include <Scene/Utils/GameObjectDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "Targeter.sgenerated.hpp"

using namespace D_SCENE;

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(Targeter);

	Targeter::Targeter() :
		D_ECS_COMP::BehaviourComponent(),
		mTargetObject()
	{ }

	Targeter::Targeter(D_CORE::Uuid uuid) :
		D_ECS_COMP::BehaviourComponent(uuid),
		mTargetObject()
	{ }

	void Targeter::Start()
	{

	}

	void Targeter::Update(float deltaTime)
	{
		if (!mTargetObject.IsValid())
			return;

		auto trans = GetTransform();
			trans->SetRotation(D_MATH::LookAt(trans->GetPosition(), mTargetObject->GetTransform()->GetPosition()));
	}

#ifdef _D_EDITOR
	bool Targeter::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();


		{
			D_H_DETAILS_DRAW_PROPERTY("Target");

			D_H_GAMEOBJECT_SELECTION_DRAW(mTargetObject, SetTarget);
			
		}


		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

	void Targeter::SetTarget(GameObject* go)
	{
		mTargetObject = GameObjectRef(go);
	}

}
