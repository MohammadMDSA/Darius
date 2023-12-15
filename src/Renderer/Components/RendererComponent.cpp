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
		mCastsShadow(true),
		mStencilValue(0u),
		mStencilWriteEnable(false)
	{
		SetDirty();
	}

	RendererComponent::RendererComponent(D_CORE::Uuid uuid) :
		D_ECS_COMP::ComponentBase(uuid),
		mCastsShadow(false),
		mStencilValue(0u),
		mStencilWriteEnable(false)
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

		// Stencil enable
		{
			bool value = IsStencilWriteEnable();
			D_H_DETAILS_DRAW_PROPERTY("Stencil Write");
			if (ImGui::Checkbox("##StencilWrite", &value))
			{
				SetStencilWriteEnable(value);
				valueChanged = true;
			}
		}

		// Stencil value
		{
			UINT8 value = GetStencilValue();
			D_H_DETAILS_DRAW_PROPERTY("Stencil Value");;
			if (ImGui::DragScalar("##StencilValue", ImGuiDataType_U8, &value))
			{
				SetStencilValue(value);
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

	void RendererComponent::SetStencilValue(UINT8 value)
	{
		if (!CanChange())
			return;

		if (mStencilValue == value)
			return;

		mStencilValue = value;

		mChangeSignal(this);
	}

	void RendererComponent::SetStencilWriteEnable(bool value)
	{
		if (!CanChange())
			return;

		if (mStencilWriteEnable == value)
			return;

		mStencilWriteEnable = value;

		mChangeSignal(this);
	}
}
