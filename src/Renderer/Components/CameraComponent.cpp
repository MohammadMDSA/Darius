#include "Renderer/pch.hpp"
#include "CameraComponent.hpp"

#include <Core/Serialization/TypeSerializer.hpp>
#include <Renderer/Camera/CameraManager.hpp>
#include <Scene/Utils/DetailsDrawer.hpp>

#ifdef _D_EDITOR
#include <imgui.h>
#endif

#include "LightComponent.sgenerated.hpp"

using namespace D_MATH;
using namespace D_SERIALIZATION;
using namespace DirectX;

namespace Darius::Graphics
{
	D_H_COMP_DEF(CameraComponent);

	CameraComponent::CameraComponent() :
		ComponentBase()
	{
		mCamera.SetFOV(XM_PI / 3);
		mCamera.SetZRange(0.001f, 10000.f);
		mCamera.SetPosition(Vector3(2.f, 2.f, 2.f));
		mCamera.SetLookDirection(Vector3(-2), Vector3::Up);
		mCamera.SetOrthographicSize(10);
		mCamera.SetOrthographic(false);
	}

	CameraComponent::CameraComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid)
	{
		mCamera.SetFOV(XM_PI / 3);
		mCamera.SetZRange(0.001f, 10000.f);
		mCamera.SetPosition(Vector3(2.f, 2.f, 2.f));
		mCamera.SetLookDirection(Vector3(-2), Vector3::Up);
		mCamera.SetOrthographicSize(10);
		mCamera.SetOrthographic(false);
	}

	void CameraComponent::Awake()
	{
		mCamera.UpdateProjMatrix();
		auto cam = D_CAMERA_MANAGER::GetActiveCamera();
		if (cam.IsValid())
			return;

		D_CAMERA_MANAGER::SetActiveCamera(*this);
	}

	void CameraComponent::OnDestroy()
	{
		auto actuve = D_CAMERA_MANAGER::GetActiveCamera();
		if (actuve == *this)
			D_CAMERA_MANAGER::SetActiveCamera(D_ECS::CompRef<CameraComponent>());
	}

	void CameraComponent::Update(float dt)
	{
		auto transform = GetTransform();
		mCamera.SetPosition(transform.Translation);
		mCamera.SetRotation(transform.Rotation);
		mCamera.Update();
	}

#ifdef _D_EDITOR
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
		D_SERIALIZATION::Serialize(*this, j);
	}

	void CameraComponent::Deserialize(Json const& j)
	{
		D_SERIALIZATION::Deserialize(*this, j);
	}

}
