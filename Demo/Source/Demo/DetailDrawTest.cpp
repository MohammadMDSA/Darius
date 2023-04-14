#include <pch.hpp>
#include "DetailDrawTest.hpp"

#include <ResourceManager/Resource.hpp>

#ifdef _D_EDITOR
#include <Editor/GUI/DetailDrawer/DetailDrawer.hpp>
#include <Utils/DragDropPayload.hpp>
#include <imgui.h>
#endif

#include "DetailDrawTest.sgenerated.hpp"

namespace Demo
{
	D_H_BEHAVIOUR_COMP_DEF(DetailDrawTest);

	DetailDrawTest::DetailDrawTest() :
		D_ECS_COMP::BehaviourComponent()
	{ }

	DetailDrawTest::DetailDrawTest(D_CORE::Uuid uuid) :
		D_ECS_COMP::BehaviourComponent(uuid)
	{ }

	void DetailDrawTest::Start()
	{

	}

	void DetailDrawTest::Update(float deltaTime)
	{
		
	}

#ifdef _D_EDITOR
	bool DetailDrawTest::DrawDetails(float params[])
	{
		return D_DETAIL_DRAWER::DrawDetials(*this);
	}
#endif

}
