#include <pch.hpp>
#include "%%CLASS_NAME%%.hpp"

#include <ResourceManager/Resource.hpp>

#ifdef _D_EDITOR
#include <Scene/Utils/GameObjectDragDropPayload.hpp>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "%%CLASS_NAME%%.sgenerated.hpp"

namespace %%NAMESPACE%%
{
	D_H_BEHAVIOUR_COMP_DEF(%%CLASS_NAME%%);

	%%CLASS_NAME%%::%%CLASS_NAME%%() :
		Super()
	{ }

	%%CLASS_NAME%%::%%CLASS_NAME%%(D_CORE::Uuid const& uuid) :
		Super(uuid)
	{ }

	void %%CLASS_NAME%%::Start()
	{

	}

	void %%CLASS_NAME%%::Update(float deltaTime)
	{
		
	}

#ifdef _D_EDITOR
	bool %%CLASS_NAME%%::DrawDetails(float params[])
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
