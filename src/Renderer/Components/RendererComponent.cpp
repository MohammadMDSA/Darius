#include "Renderer/pch.hpp"
#include "RendererComponent.hpp"

#ifdef _D_EDITOR
#include <Libs/FontIcon/IconsFontAwesome6.h>
#include <imgui.h>
#endif

#include <RendererComponent.sgenerated.hpp>

namespace Darius::Renderer
{
	D_H_COMP_DEF(RendererComponent);

	RendererComponent::RendererComponent() :
		D_ECS_COMP::ComponentBase(),
		mCastsShadow(true)
	{
		SetDirty();
	}

	RendererComponent::RendererComponent(D_CORE::Uuid uuid) :
		D_ECS_COMP::ComponentBase(uuid),
		mCastsShadow(false)
	{
		SetDirty();
	}

#ifdef _D_EDITOR
	bool RendererComponent::DrawDetails(float[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE("RendererComponent");


		// Castting shadow
		{
			bool value = IsCastingShadow();
			D_H_DETAILS_DRAW_PROPERTY("Casts Shadow");
			if (ImGui::Checkbox("##CastsShadow", &value))
			{
				SetCastsShadow(value);
				valueChanged = true;
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif


	void RendererComponent::SetCastsShadow(bool value)
	{
		if (!CanChange())
			return;

		if (mCastsShadow == value)
			return;

		mCastsShadow = value;

		mChangeSignal(this);
	}

}
