#include "Renderer/pch.hpp"
#include "RendererComponent.hpp"

#include "Renderer/RendererManager.hpp"

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
		mStencilWriteEnable(false),
		mCustomDepthEnable(false)
	{
		SetDirty();
	}

	RendererComponent::RendererComponent(D_CORE::Uuid const& uuid) :
		D_ECS_COMP::ComponentBase(uuid),
		mCastsShadow(false),
		mStencilValue(0u),
		mStencilWriteEnable(false),
		mCustomDepthEnable(false)
	{
		SetDirty();
	}

	void RendererComponent::Awake()
	{

		// Setting up picker draw ps constants
#if _D_EDITOR
		D_STATIC_ASSERT(sizeof(DirectX::XMUINT2) == sizeof(uint64_t));

		uint64_t entityId = GetGameObject()->GetEntity().id();
		PickerPsMeshConstants constants;
		std::memcpy(&constants.Id, &entityId, sizeof(entityId));
		mPickerDrawPsConstant.Create(L"Picker PS Mesh Constants", 1u, sizeof(PickerPsMeshConstants), &constants);
#endif // _D_EDITOR
	}

	void RendererComponent::Start()
	{
		if(!mBvhNodeId.IsValid())
		{
			mBvhNodeId = D_RENDERER::RegisterComponent(D_ECS::UntypedCompRef(GetGameObject()->GetEntity(), GetComponentEntry()));
		}
		SetDirty();
	}

	void RendererComponent::Update(float dt)
	{
		if(mBvhNodeId.IsValid())
			D_RENDERER::UpdateComponentBounds(mBvhNodeId, GetAabb());
	}

	void RendererComponent::OnActivate()
	{
		if(!mBvhNodeId.IsValid())
		{
			mBvhNodeId = D_RENDERER::RegisterComponent(D_ECS::UntypedCompRef(GetGameObject()->GetEntity(), GetComponentEntry()));
		}
	}

	void RendererComponent::OnDeactivate()
	{
		if(mBvhNodeId.IsValid())
		{
			D_RENDERER::UnregisterComponent(mBvhNodeId);
			mBvhNodeId = {};
		}
	}

	void RendererComponent::OnDestroy()
	{
#if _D_EDITOR
		mPickerDrawPsConstant.Destroy();
#endif // _D_EDITOR
		
		if(mBvhNodeId.IsValid())
		{
			D_RENDERER::UnregisterComponent(mBvhNodeId);
			mBvhNodeId = {};
		}
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
			if(ImGui::Checkbox("##CastsShadow", &value))
			{
				SetCastsShadow(value);
				valueChanged = true;
			}
		}

		// Stencil enable
		{
			bool value = IsStencilWriteEnable();
			D_H_DETAILS_DRAW_PROPERTY("Stencil Write");
			if(ImGui::Checkbox("##StencilWrite", &value))
			{
				SetStencilWriteEnable(value);
				valueChanged = true;
			}
		}

		// Stencil value
		{
			UINT8 value = GetStencilValue();
			D_H_DETAILS_DRAW_PROPERTY("Stencil Value");
			if(ImGui::DragScalar("##StencilValue", ImGuiDataType_U8, &value))
			{
				SetStencilValue(value);
				valueChanged = true;
			}
		}

		// Custom Depth
		{
			bool value = IsCustomDepthEnable();
			D_H_DETAILS_DRAW_PROPERTY("Custom Depth");
			if(ImGui::Checkbox("##CustomDepth", &value))
			{
				SetCustomDepthEnable(value);
				valueChanged = true;
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}

#endif


	void RendererComponent::SetCastsShadow(bool value)
	{
		if(!CanChange())
			return;

		if(mCastsShadow == value)
			return;

		mCastsShadow = value;

		mChangeSignal(this);
	}

	void RendererComponent::SetStencilValue(UINT8 value)
	{
		if(!CanChange())
			return;

		if(mStencilValue == value)
			return;

		mStencilValue = value;

		mChangeSignal(this);
	}

	void RendererComponent::SetStencilWriteEnable(bool value)
	{
		if(!CanChange())
			return;

		if(mStencilWriteEnable == value)
			return;

		mStencilWriteEnable = value;

		mChangeSignal(this);
	}

	void RendererComponent::SetCustomDepthEnable(bool value)
	{
		if(!CanChange())
			return;

		if(mCustomDepthEnable == value)
			return;

		mCustomDepthEnable = value;

		mChangeSignal(this);
	}
}
