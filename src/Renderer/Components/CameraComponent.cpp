#include "Renderer/pch.hpp"
#include "CameraComponent.hpp"

#include <Renderer/Camera/CameraManager.hpp>
#include <Scene/Utils/Serializer.hpp>
#include <Scene/Utils/DetailsDrawer.hpp>

#include <imgui.h>

using namespace D_LIGHT;

namespace Darius::Graphics
{
	D_H_COMP_DEF(CameraComponent);

	D_H_COMP_DEFAULT_CONSTRUCTOR_DEF(CameraComponent)

	void CameraComponent::Awake()
	{
		auto cam = D_CAMERA_MANAGER::GetActiveCamera();
		if (cam.IsValid())
			return;

		D_CAMERA_MANAGER::SetActiveCamera(*this);
	}

	void CameraComponent::OnDestroy()
	{

	}

#ifdef _DEBUG
	bool CameraComponent::DrawDetails(float params[])
	{
		bool changed = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE()

		D_H_DETAILS_DRAW_END_TABLE();

		return changed;
	}
#endif

	void CameraComponent::Serialize(Json& j) const
	{	
	}

	void CameraComponent::Deserialize(Json const& j)
	{
	}

}
