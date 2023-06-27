#include "Renderer/pch.hpp"
#include "CameraComponent.hpp"

#include "Renderer/Camera/CameraManager.hpp"

#include <Debug/DebugDraw.hpp>
#include <Scene/Utils/DetailsDrawer.hpp>

#ifdef _D_EDITOR
#include <imgui.h>
#endif

#include "CameraComponent.sgenerated.hpp"

using namespace D_MATH;
using namespace D_SERIALIZATION;
using namespace DirectX;

namespace Darius::Renderer
{
	D_H_COMP_DEF(CameraComponent);

	CameraComponent::CameraComponent() :
		ComponentBase()
	{
		mCamera.SetFoV(XM_PI / 3);
		mCamera.SetZRange(0.001f, 10000.f);
		mCamera.SetPosition(Vector3(2.f, 2.f, 2.f));
		mCamera.SetLookDirection(Vector3(-2), Vector3::Up);
		mCamera.SetOrthographicSize(10);
		mCamera.SetOrthographic(false);
	}

	CameraComponent::CameraComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid)
	{
		mCamera.SetFoV(XM_PI / 3);
		mCamera.SetZRange(0.001f, 10000.f);
		mCamera.SetPosition(Vector3(2.f, 2.f, 2.f));
		mCamera.SetLookDirection(Vector3(-2), Vector3::Up);
		mCamera.SetOrthographicSize(10);
		mCamera.SetOrthographic(false);
	}

	void CameraComponent::Awake()
	{
		mCamera.UpdateProjMatrix();

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
		mCamera.SetPosition(transform->GetPosition());
		mCamera.SetRotation(transform->GetRotation());
		mCamera.Update();

		D_DEBUG_DRAW::DrawFrustum(mCamera.GetWorldSpaceFrustum());
	}

#ifdef _D_EDITOR
	bool CameraComponent::DrawDetails(float params[])
	{
		bool changed = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		{
			D_H_DETAILS_DRAW_PROPERTY("FoV");
			float fov = DirectX::XMConvertToDegrees(mCamera.GetFoV());
			if (ImGui::SliderFloat("##FoV", &fov, 1.f, 89.f))
			{
				mCamera.SetFoV(DirectX::XMConvertToRadians(fov));
				changed = true;
			}
		}

		{
			D_H_DETAILS_DRAW_PROPERTY("Near Clip");
			float nearClip = GetNearClip();
			if (ImGui::DragFloat("##NearClip", &nearClip, 0.01f, 0.001f, GetFarClip()))
			{
				SetNearClip(nearClip);
				changed = true;
			}
		}

		if(!IsInfiniteZ())
		{
			D_H_DETAILS_DRAW_PROPERTY("Far Clip");
			float farClip = GetFarClip();
			if (ImGui::DragFloat("##FarClip", &farClip, 1.f, GetNearClip(), FLT_MAX))
			{
				SetFarClip(farClip);
				changed = true;
			}
		}

		if(IsOrthographic())
		{
			D_H_DETAILS_DRAW_PROPERTY("Ortho Size");
			float orthoSize = GetOrthographicSize();
			if (ImGui::DragFloat("##OrthoSize", &orthoSize, 1.f, FLT_MAX))
			{
				SetOrthographicSize(orthoSize);
				changed = true;
			}
		}

		{
			D_H_DETAILS_DRAW_PROPERTY("Infinite Z");
			bool isInf = IsInfiniteZ();
			if (ImGui::Checkbox("##InfiniteZ", &isInf))
			{
				SetInfiniteZ(isInf);
				changed = true;
			}
		}

		{
			D_H_DETAILS_DRAW_PROPERTY("Orthographic");
			bool isOrtho = IsOrthographic();
			if (ImGui::Checkbox("##Orthographic", &isOrtho))
			{
				SetOrthographic(isOrtho);
				changed = true;
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return changed;
	}
#endif

}
