#include "Graphics/pch.hpp"
#include "LightComponent.hpp"

#include "Graphics/Camera/CameraManager.hpp"

#include <Math/Serialization.hpp>
#include <Scene/Utils/DetailsDrawer.hpp>
#include <Debug/DebugDraw.hpp>

#include "LightComponent.sgenerated.hpp"

#ifdef _D_EDITOR
#include <imgui.h>
#endif

using namespace D_LIGHT;
using namespace D_SERIALIZATION;
using namespace DirectX;

namespace Darius::Graphics
{
	D_H_COMP_DEF(LightComponent);

	LightComponent::LightComponent() :
		D_ECS_COMP::ComponentBase(),
		mLightType(LightSourceType::PointLight),
		mConeInnerAngle(XMConvertToRadians(30)),
		mConeOuterAngle(XMConvertToRadians(45))
	{
		UpdateAngleData();
	}

	LightComponent::LightComponent(D_CORE::Uuid uuid) :
		D_ECS_COMP::ComponentBase(uuid),
		mLightType(LightSourceType::PointLight),
		mConeInnerAngle(XMConvertToRadians(30)),
		mConeOuterAngle(XMConvertToRadians(45))
	{
		UpdateAngleData();
	}

	void LightComponent::Awake()
	{
		UpdateAngleData();
	}

#ifdef _D_EDITOR
	bool LightComponent::DrawDetails(float params[])
	{
		bool changed = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE()

			// Light type
			D_H_DETAILS_DRAW_PROPERTY("Light Type");
		int lightType = (int)mLightType;
		if (ImGui::Combo("##LightTyp", &lightType, "DirectionalLight\0PointLight\0SpotLight\0\0"))
		{
			SetLightType((LightSourceType)lightType);
			changed = true;
		}

		// Light type
		D_H_DETAILS_DRAW_PROPERTY("Color");
		float defC[] = D_H_DRAW_DETAILS_MAKE_VEC_PARAM_COLOR;
		changed |= D_MATH::DrawDetails(*(D_MATH::Vector3*)&mLightData.Color, defC);

		if (mLightType == LightSourceType::SpotLight)
		{
			bool anglesChanged = false;

			auto innerDeg = XMConvertToDegrees(mConeInnerAngle);
			auto outerDeg = XMConvertToDegrees(mConeOuterAngle);

			// Spot inner
			D_H_DETAILS_DRAW_PROPERTY("Inner Half Angle");
			if (ImGui::SliderAngle("##SpotInnerAngle", &mConeInnerAngle, 0, outerDeg))
			{
				changed = anglesChanged = true;
			}

			D_H_DETAILS_DRAW_PROPERTY("Outer Half Angle");
			if (ImGui::SliderAngle("##SpotOuterAngle", &mConeOuterAngle, innerDeg, 79.f))
				changed = anglesChanged = true;

			if (anglesChanged)
				UpdateAngleData();
		}

		D_H_DETAILS_DRAW_PROPERTY("Intencity");
		changed |= ImGui::DragFloat("##Intencity", &mLightData.Intencity, 0.01f, 0.f, FLT_MAX, "%.3f");

		if (mLightType != LightSourceType::DirectionalLight)
		{
			D_H_DETAILS_DRAW_PROPERTY("Range");
			changed |= ImGui::DragFloat("##Range", &mLightData.Range, 0.01f, mLightData.Intencity, -1.f, "%.3f");
		}

		D_H_DETAILS_DRAW_PROPERTY("Casts Shadow");
		bool val = mLightData.CastsShadow;
		if (ImGui::Checkbox("##CastsShadow", &val))
		{
			mLightData.CastsShadow = val ? 1 : 0;
			changed = true;
		}


		D_H_DETAILS_DRAW_END_TABLE();

		return changed;
	}

	void LightComponent::OnGizmo() const
	{
		auto trans = GetTransform();

		switch (mLightType)
		{
		case Darius::Renderer::LightManager::LightSourceType::DirectionalLight:
		{
			auto activeCam = D_CAMERA_MANAGER::GetActiveCamera();

			if (!activeCam.IsValid())
				break;

			auto const& frustom = activeCam->GetCamera().GetWorldSpaceFrustum();

			// Creating view mat regardless of its position
			auto invLightViewMat = D_MATH::OrthogonalTransform(trans->GetRotation(), D_MATH::Vector3(D_MATH::kZero));
			auto lightViewMat = D_MATH::Invert(invLightViewMat);

			// Creating shadow aabb
			D_MATH_BOUNDS::AxisAlignedBox shadowAABB;
			for (UINT i = 0; i < D_MATH_CAMERA::Frustum::_kNumCorners; i++)
			{
				auto corner = frustom.GetFrustumCorner((D_MATH_CAMERA::Frustum::CornerID)i);
				shadowAABB.AddPoint(lightViewMat * corner);
			}

			auto shadowAABBMin = shadowAABB.GetMin();
			auto shadowAABBMax = shadowAABB.GetMax();
			D_CONTAINERS::DVector<D_MATH::Vector3> verts;

#define ADD_TO_VERT_VEC(...) \
			verts.push_back(invLightViewMat * D_MATH::Vector3(__VA_ARGS__));

			ADD_TO_VERT_VEC(shadowAABBMin.GetX(), shadowAABBMin.GetY(), shadowAABBMax.GetZ());
			ADD_TO_VERT_VEC(shadowAABBMin.GetX(), shadowAABBMax.GetY(), shadowAABBMax.GetZ());
			ADD_TO_VERT_VEC(shadowAABBMax.GetX(), shadowAABBMin.GetY(), shadowAABBMax.GetZ());
			ADD_TO_VERT_VEC(shadowAABBMax.GetX(), shadowAABBMax.GetY(), shadowAABBMax.GetZ());
			ADD_TO_VERT_VEC(shadowAABBMin.GetX(), shadowAABBMin.GetY(), shadowAABBMin.GetZ());
			ADD_TO_VERT_VEC(shadowAABBMin.GetX(), shadowAABBMax.GetY(), shadowAABBMin.GetZ());
			ADD_TO_VERT_VEC(shadowAABBMax.GetX(), shadowAABBMin.GetY(), shadowAABBMin.GetZ());
			ADD_TO_VERT_VEC(shadowAABBMax.GetX(), shadowAABBMax.GetY(), shadowAABBMin.GetZ());
#undef ADD_TO_VERT_VEC

			// Draw box from verts
			D_DEBUG_DRAW::DrawCubeLines(verts, 0.f, D_MATH::Color(mLightData.Color.x, mLightData.Color.y, mLightData.Color.z));

			break;
		}
		case Darius::Renderer::LightManager::LightSourceType::PointLight:
		{
			D_DEBUG_DRAW::DrawSphere(trans->GetPosition(), mLightData.Range, 0.f, D_MATH::Color(mLightData.Color.x, mLightData.Color.y, mLightData.Color.z));
			break;
		}
		case Darius::Renderer::LightManager::LightSourceType::SpotLight:
			auto innerAng = mConeInnerAngle == 0 ? 0.001f : mConeInnerAngle;
			auto outerAng = mConeOuterAngle == 0 ? 0.001f : mConeOuterAngle;
			// Drawing outer cone
			D_DEBUG_DRAW::DrawConeLines(trans->GetPosition(), trans->GetRotation().GetForward(), D_MATH::Cos(outerAng) * mLightData.Range, D_MATH::Sin(outerAng) * mLightData.Range, 0.f, D_MATH::Color(mLightData.Color.x, mLightData.Color.y, mLightData.Color.z, 0.6f));
			// Drawing inner cone
			D_DEBUG_DRAW::DrawConeLines(trans->GetPosition(), trans->GetRotation().GetForward(), D_MATH::Cos(innerAng) * mLightData.Range, D_MATH::Sin(innerAng) * mLightData.Range, 0.f, D_MATH::Color(mLightData.Color.x, mLightData.Color.y, mLightData.Color.z));
			break;
		}
	}

#endif

}
