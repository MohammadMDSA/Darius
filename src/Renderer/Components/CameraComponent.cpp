#include "Renderer/pch.hpp"
#include "CameraComponent.hpp"

#include "Renderer/Camera/CameraManager.hpp"
#include "Renderer/Rasterization/Light/ShadowedLightContext.hpp"
#include "Renderer/RendererManager.hpp"

#include <Debug/DebugDraw.hpp>
#include <Scene/Utils/DetailsDrawer.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

#ifdef _D_EDITOR
#include <ResourceManager/ResourceDragDropPayload.hpp>
#include <Libs/FontIcon/IconsFontAwesome6.h>
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

	CameraComponent::CameraComponent(D_CORE::Uuid const& uuid) :
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

		if (D_RENDERER::GetActiveRendererType() == D_RENDERER::RendererType::Rasterization)
		{
			D_CONTAINERS::DVector<D_MATH_CAMERA::Frustum> frustums;
			mCamera.CalculateSlicedFrustumes(D_RENDERER_RAST::GetLightContext()->GetCascades(), frustums);

			for (auto& frust : frustums)
				D_DEBUG_DRAW::DrawFrustum(frust);
		}
		else
			D_DEBUG_DRAW::DrawFrustum(mCamera.GetWorldSpaceFrustum());
	}

#ifdef _D_EDITOR
	bool CameraComponent::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		{
			D_H_DETAILS_DRAW_PROPERTY("FoV");
			float fov = DirectX::XMConvertToDegrees(mCamera.GetFoV());
			if (ImGui::SliderFloat("##FoV", &fov, 1.f, 89.f))
			{
				mCamera.SetFoV(DirectX::XMConvertToRadians(fov));
				valueChanged = true;
			}
		}

		{
			D_H_DETAILS_DRAW_PROPERTY("Near Clip");
			float nearClip = GetNearClip();
			if (ImGui::DragFloat("##NearClip", &nearClip, 0.01f, 0.001f, GetFarClip()))
			{
				SetNearClip(nearClip);
				valueChanged = true;
			}
		}

		if (!IsInfiniteZ())
		{
			D_H_DETAILS_DRAW_PROPERTY("Far Clip");
			float farClip = GetFarClip();
			if (ImGui::DragFloat("##FarClip", &farClip, 1.f, GetNearClip(), FLT_MAX))
			{
				SetFarClip(farClip);
				valueChanged = true;
			}
		}

		if (IsOrthographic())
		{
			D_H_DETAILS_DRAW_PROPERTY("Ortho Size");
			float orthoSize = GetOrthographicSize();
			if (ImGui::DragFloat("##OrthoSize", &orthoSize, 1.f, FLT_MAX))
			{
				SetOrthographicSize(orthoSize);
				valueChanged = true;
			}
		}

		{
			D_H_DETAILS_DRAW_PROPERTY("Infinite Z");
			bool isInf = IsInfiniteZ();
			if (ImGui::Checkbox("##InfiniteZ", &isInf))
			{
				SetInfiniteZ(isInf);
				valueChanged = true;
			}
		}

		{
			D_H_DETAILS_DRAW_PROPERTY("Orthographic");
			bool isOrtho = IsOrthographic();
			if (ImGui::Checkbox("##Orthographic", &isOrtho))
			{
				SetOrthographic(isOrtho);
				valueChanged = true;
			}
		}

		{
			D_H_DETAILS_DRAW_PROPERTY("Skybox Specular (Color)");
			D_H_RESOURCE_SELECTION_DRAW(D_RENDERER::TextureResource, mSkyboxSpecular, "Select Specular Texture", SetSkyboxSpecularTexture);
		}

		{
			D_H_DETAILS_DRAW_PROPERTY("Skybox Diffuse (Light)");
			D_H_RESOURCE_SELECTION_DRAW(D_RENDERER::TextureResource, mSkyboxDiffuse, "Select Diffuse Texture", SetSkyboxDiffuseTexture);
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif

	void CameraComponent::SetFoV(float verticalFov)
	{
		verticalFov = D_MATH::Clamp(verticalFov, 1.f, 179.f);

		if(mCamera.GetFoV() == verticalFov)
			return;

		mCamera.SetFoV(verticalFov);

		mChangeSignal(this);
	}

	void CameraComponent::SetNearClip(float nearClip)
	{
		nearClip = D_MATH::Max(nearClip, 0.01f);

		if(mCamera.GetNearClip() == nearClip)
			return;

		mCamera.SetZRange(nearClip, mCamera.GetFarClip());

		mChangeSignal(this);
	}

	void CameraComponent::SetFarClip(float farClip)
	{
		farClip = D_MATH::Max(farClip, 0.01f);

		if(mCamera.GetFarClip() == farClip)
			return;

		mCamera.SetZRange(GetNearClip(), farClip);

		mChangeSignal(this);
	}

	void CameraComponent::SetOrthographicSize(float size)
	{
		size = D_MATH::Max(size, 0.f);
		
		if(mCamera.GetOrthographicSize() == size)
			return;

		mCamera.SetOrthographicSize(size);

		mChangeSignal(this);
	}

	void CameraComponent::SetAspectRatio(float ratio)
	{
		ratio = D_MATH::Max(0.001f, ratio);

		if(mCamera.GetAspectRatio() == ratio)
			return;

		mCamera.SetAspectRatio(ratio);

		mChangeSignal(this);
	}

	void CameraComponent::SetInfiniteZ(bool infinite)
	{
		if(mCamera.IsInfiniteZ() == infinite)
			return;

		mCamera.SetInfiniteZ(infinite);

		mChangeSignal(this);
	}

	void CameraComponent::SetOrthographic(bool ortho)
	{
		if(mCamera.IsOrthographic() == ortho)
			return;

		mCamera.SetOrthographic(ortho);

		mChangeSignal(this);
	}

	void CameraComponent::SetSkyboxSpecularTexture(D_RENDERER::TextureResource* texture)
	{
		if(mSkyboxSpecular == texture)
			return;

		if(texture && !texture->GetTextureData()->IsCubeMap())
		{
			D_LOG_WARN("Only Cube Map texture type is supported of skybox specular.");
			return;
		}

		mSkyboxSpecular = texture;

		if(mSkyboxSpecular.IsValid() && !mSkyboxSpecular->IsLoaded())
			D_RESOURCE_LOADER::LoadResourceAsync(mSkyboxSpecular.Get(), nullptr, true);

		mChangeSignal(this);

	}

	void CameraComponent::SetSkyboxDiffuseTexture(D_RENDERER::TextureResource* texture)
	{
		if(mSkyboxDiffuse == texture)
			return;

		if(texture && !texture->GetTextureData()->IsCubeMap())
		{
			D_LOG_WARN("Only Cube Map texture type is supported of skybox diffuse.");
			return;
		}

		mSkyboxDiffuse = texture;

		if(mSkyboxDiffuse.IsValid() && !mSkyboxDiffuse->IsLoaded())
			D_RESOURCE_LOADER::LoadResourceAsync(mSkyboxDiffuse.Get(), nullptr, true);

		mChangeSignal(this);
	}

}
