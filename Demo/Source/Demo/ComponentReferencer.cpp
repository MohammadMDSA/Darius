#include <pch.hpp>
#include "ComponentReferencer.hpp"

#include <ResourceManager/Resource.hpp>

#ifdef _D_EDITOR
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "ComponentReferencer.sgenerated.hpp"

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(ComponentReferencer);

	ComponentReferencer::ComponentReferencer() :
		D_ECS_COMP::BehaviourComponent()
	{ }

	ComponentReferencer::ComponentReferencer(D_CORE::Uuid const& uuid) :
		D_ECS_COMP::BehaviourComponent(uuid)
	{ }

	void ComponentReferencer::Start()
	{
		
	}

	void ComponentReferencer::Update(float deltaTime)
	{
		if (!mOtherTrans.IsValid())
			return;

		auto trans = GetTransform();
		trans->SetRotation(D_MATH::LookAt(trans->GetPosition(), mOtherTrans->GetPosition()));
	}

	void ComponentReferencer::SetOtherTrans(D_MATH::TransformComponent* other)
	{
		if (mOtherTrans == other)
			return;

		mOtherTrans = other;

		mChangeSignal(this);
	}

	void ComponentReferencer::SetAnotherRef(ComponentReferencer* other)
	{
		if (mAnotherRef == other)
			return;

		mAnotherRef = other;

		mChangeSignal(this);
	}


#ifdef _D_EDITOR
	bool ComponentReferencer::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Other Comp Ref
		{
			D_H_DETAILS_DRAW_PROPERTY("Other Trans");

			D_H_COMPONENT_SELECTION_DRAW(D_MATH::TransformComponent, mOtherTrans, SetOtherTrans);
		}

		// Another Comp Ref
		{
			D_H_DETAILS_DRAW_PROPERTY("Another ref");

			D_H_COMPONENT_SELECTION_DRAW(ComponentReferencer, mAnotherRef, SetAnotherRef);
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

}
