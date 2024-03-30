#include "Physics/pch.hpp"
#include "BoxColliderComponent.hpp"

#include <Renderer/Components/MeshRendererComponent.hpp>
#include <Renderer/Components/SkeletalMeshRendererComponent.hpp>
#include <Debug/DebugDraw.hpp>

#if _D_EDITOR
#include <imgui.h>
#endif

#include "BoxColliderComponent.sgenerated.hpp"

using namespace D_RENDERER;
using namespace D_MATH;

namespace Darius::Physics
{

	D_H_COMP_DEF(BoxColliderComponent);

	BoxColliderComponent::BoxColliderComponent() :
		ColliderComponent(),
		mHalfExtents(0.5f),
		mScaledHalfExtents(0.5f)
	{
		SetDirty();
	}

	BoxColliderComponent::BoxColliderComponent(D_CORE::Uuid const& uuid) :
		ColliderComponent(uuid),
		mHalfExtents(0.5f),
		mScaledHalfExtents(0.5f)
	{
		SetDirty();
	}

#ifdef _D_EDITOR
	void BoxColliderComponent::OnPostComponentAddInEditor()
	{
		Super::OnPostComponentAddInEditor();

		bool addedDefault = false;
		if (auto sk = GetGameObject()->GetComponent<D_RENDERER::SkeletalMeshRendererComponent>())
		{
			auto mesh = sk->GetMesh();
			if (mesh && mesh->IsLoaded())
			{
				auto meshData = mesh->GetMeshData();
				auto const& boundingBox = meshData->mBoundBox;
				SetCenterOffset(boundingBox.GetCenter());
				SetHalfExtents(boundingBox.GetExtents());

				addedDefault = true;
			}
		}

		if (!addedDefault)
		{
			if (auto sm = GetGameObject()->GetComponent<D_RENDERER::MeshRendererComponent>())
			{
				auto mesh = sm->GetMesh();
				if (mesh && mesh->IsLoaded())
				{
					auto meshData = mesh->GetMeshData();
					auto const& boundingBox = meshData->mBoundBox;
					SetCenterOffset(boundingBox.GetCenter());
					SetHalfExtents(boundingBox.GetExtents());

					addedDefault = true;
				}
			}
		}
	}

	bool BoxColliderComponent::DrawDetails(float params[])
	{
		bool valueChanged = false;

		valueChanged |= ColliderComponent::DrawDetails(params);

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Half extents
		{
			D_H_DETAILS_DRAW_PROPERTY("Half Extents");
			auto halfExt = GetHalfExtents();
			if (D_MATH::DrawDetails(halfExt, {0.5f, 0.5f, 0.5f}, false))
			{
				SetHalfExtents(halfExt);
				valueChanged = true;
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}

	void BoxColliderComponent::OnGizmo() const
	{
		if (!IsActive())
			return;
		auto transform = GetTransform();
		auto rot = transform->GetRotation();
		auto offset = rot * GetScaledCenterOffset();
		D_DEBUG_DRAW::DrawCube(transform->GetPosition() + offset, transform->GetRotation(), mScaledHalfExtents * 2, 0, {0.f, 1.f, 0.f, 1.f});
	}

#endif

	bool BoxColliderComponent::CalculateGeometry(physx::PxGeometry& geom) const
	{
		physx::PxBoxGeometry& box = reinterpret_cast<physx::PxBoxGeometry&>(geom);
		box = physx::PxBoxGeometry(D_PHYSICS::GetVec3(mScaledHalfExtents));

		return true;
	}

	void BoxColliderComponent::CalculateScaledParameters()
	{
		auto scale = GetTransform()->GetScale();

		// Calc scaled half extend
		{
			auto tmp = scale * GetHalfExtents();
			mScaledHalfExtents = D_MATH::Max(D_MATH::Abs(tmp), D_MATH::Vector3(MinExtent));
		}

		Super::CalculateScaledParameters();
	}

	void BoxColliderComponent::SetHalfExtents(D_MATH::Vector3 const& halfExtents)
	{
		if (mHalfExtents == halfExtents)
			return;

		mHalfExtents = halfExtents;
		SetDirty();
		
		mChangeSignal(this);
	}

}
