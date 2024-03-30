#include <pch.hpp>
#include "GameObjectReferencer.hpp"

#include <ResourceManager/Resource.hpp>

#ifdef _D_EDITOR
#include <Scene/Utils/GameObjectDragDropPayload.hpp>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "GameObjectReferencer.sgenerated.hpp"

using namespace D_SCENE;

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(GameObjectReferencer);

	GameObjectReferencer::GameObjectReferencer() :
		D_ECS_COMP::BehaviourComponent(),
		mReference()
	{ }

	GameObjectReferencer::GameObjectReferencer(D_CORE::Uuid const& uuid) :
		D_ECS_COMP::BehaviourComponent(uuid),
		mReference()
	{ }

	void GameObjectReferencer::Start()
	{

	}

	void GameObjectReferencer::Update(float deltaTime)
	{
		
	}

#ifdef _D_EDITOR
	bool GameObjectReferencer::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();


		// sample field
		D_H_DETAILS_DRAW_PROPERTY("Reference");

		D_H_GAMEOBJECT_SELECTION_DRAW(mReference, SetReference);

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

	void GameObjectReferencer::SetReference(GameObject* go)
	{
		mReference = GameObjectRef(go);
	}

}
