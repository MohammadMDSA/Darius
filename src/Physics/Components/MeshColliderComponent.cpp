#include "Physics/pch.hpp"
#include "MeshColliderComponent.hpp"

#include <Debug/DebugDraw.hpp>

#if _D_EDITOR
#include <imgui.h>
#endif

#include "MeshColliderComponent.sgenerated.hpp"

using namespace physx;

namespace Darius::Physics
{

	D_H_COMP_DEF(MeshColliderComponent);

	MeshColliderComponent::MeshColliderComponent() :
		ColliderComponent()
	{
		SetDirty();
	}

	MeshColliderComponent::MeshColliderComponent(D_CORE::Uuid uuid) :
		ColliderComponent(uuid)
	{
		SetDirty();
	}

#ifdef _D_EDITOR

	bool MeshColliderComponent::DrawDetails(float params[])
	{
		bool valueChanged = false;
		
		valueChanged |= Super::DrawDetails(params);

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Is Convex
		{
			ImGui::BeginDisabled(true);

			D_H_DETAILS_DRAW_PROPERTY("Convex");
			bool convex = true;
			if (ImGui::Checkbox("##Convex", &convex))
			{
				valueChanged = true;
			}

			ImGui::EndDisabled();
		}

		// Tight bounds
		{
			bool value = HasTightBounds();
			D_H_DETAILS_DRAW_PROPERTY("Tight Bounds");
			if (ImGui::Checkbox("##TightBounds", &value))
			{
				SetTightBounds(value);
				valueChanged = true;
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}

	void MeshColliderComponent::OnGizmo() const
	{
		if (!IsActive())
			return;

		// Draw wireframed mesh
	}

#endif // _D_EDITOR

	void MeshColliderComponent::CalculateGeometry(physx::PxGeometry& geom) const
	{
		D_ASSERT(mMesh);

		PxConvexMeshGeometry& convMesh = reinterpret_cast<physx::PxConvexMeshGeometry&>(geom);

		PxMeshScale scale(D_PHYSICS::GetVec3(GetUsedScale()));

		PxConvexMeshGeometryFlags flags;
		if (mTightBounds)
			flags |= PxConvexMeshGeometryFlag::eTIGHT_BOUNDS;

		convMesh = physx::PxConvexMeshGeometry(mMesh, scale, flags);
	}

	void MeshColliderComponent::SetTightBounds(bool tight)
	{
		if (mTightBounds = tight)
			return;

		mTightBounds = tight;
		SetDirty();

		mChangeSignal(this);
	}

}
