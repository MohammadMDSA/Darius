#include "Physics/pch.hpp"
#include "MeshColliderComponent.hpp"

#include <Debug/DebugDraw.hpp>

#if _D_EDITOR
#include <Libs/FontIcon/IconsFontAwesome6.h>
#include <ResourceManager/ResourceDragDropPayload.hpp>
#include <imgui.h>
#endif

#include "MeshColliderComponent.sgenerated.hpp"

using namespace physx;
using namespace D_RENDERER;

namespace Darius::Physics
{

	D_H_COMP_DEF(MeshColliderComponent);

	MeshColliderComponent::MeshColliderComponent() :
		ColliderComponent(),
		mMesh(nullptr)
	{
		SetDirty();
	}

	MeshColliderComponent::MeshColliderComponent(D_CORE::Uuid uuid) :
		ColliderComponent(uuid),
		mMesh(nullptr)
	{
		SetDirty();
	}

	void MeshColliderComponent::Awake()
	{
		if (mReferenceMesh.IsValid())
			CalculateMeshGeometry();

		Super::Awake();
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

		// Reference Mesh
		{
			D_H_DETAILS_DRAW_PROPERTY("Reference Mesh");
			D_H_RESOURCE_SELECTION_DRAW(StaticMeshResource, mReferenceMesh, "Select Mesh", SetReferenceMesh);
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}

	void MeshColliderComponent::OnGizmo() const
	{
		if (!IsActive())
			return;

		auto trans = GetTransform();
		D_DEBUG_DRAW::DrawMesh(trans->GetPosition(), trans->GetRotation(), trans->GetScale(), mDebugMesh, 0., { 0.f, 1.f, 0.f, 1.f });
	}

#endif // _D_EDITOR

	bool MeshColliderComponent::CalculateGeometry(physx::PxGeometry& geom) const
	{
		if (!mMesh)
			return false;

		PxConvexMeshGeometry& convMesh = reinterpret_cast<physx::PxConvexMeshGeometry&>(geom);

		PxMeshScale scale(D_PHYSICS::GetVec3(GetUsedScale()));

		PxConvexMeshGeometryFlags flags;
		if (mTightBounds)
			flags |= PxConvexMeshGeometryFlag::eTIGHT_BOUNDS;

		convMesh = physx::PxConvexMeshGeometry(mMesh, scale, flags);

		return true;
	}

	void MeshColliderComponent::SetTightBounds(bool tight)
	{
		if (mTightBounds = tight)
			return;

		mTightBounds = tight;
		SetDirty();

		mChangeSignal(this);
	}

	void MeshColliderComponent::SetReferenceMesh(D_RENDERER::StaticMeshResource* staticMesh)
	{

		if (mReferenceMesh == staticMesh)
			return;

		mReferenceMesh = staticMesh;

		auto meshValid = mReferenceMesh.IsValid();

		if (meshValid && mReferenceMesh->IsLoaded())
		{
			// Mesh is already loaded
			CalculateMeshGeometry();
		}
		else if (meshValid)
		{
			// Mesh needs to be loaded
			D_RESOURCE_LOADER::LoadResourceAsync(mReferenceMesh.Get(), [&](auto _)
				{
					CalculateMeshGeometry();
				}, true);
		}

		SetDirty();

		mChangeSignal(this);
	}

	void MeshColliderComponent::CalculateMeshGeometry()
	{
		if (!mReferenceMesh.IsValid())
			mMesh = nullptr;

		D_ASSERT(mReferenceMesh.IsValid());
		D_ASSERT(mReferenceMesh->IsLoaded());

		
		auto& context = D_GRAPHICS::CommandContext::Begin(L"Convex Mesh Creation Mesh Readback");

		auto meshData = mReferenceMesh->GetMeshData();
		auto& vertBuffer = const_cast<D_GRAPHICS_BUFFERS::StructuredBuffer&>(meshData->VertexDataGpu);
		auto& indexBuffer = const_cast<D_GRAPHICS_BUFFERS::StructuredBuffer&>(meshData->IndexDataGpu);

		mMeshVerticesReadback.Create(L"Convex Mesh Creation Vertices Readback", vertBuffer.GetElementCount(), vertBuffer.GetElementSize());
		context.CopyBuffer(mMeshVerticesReadback, vertBuffer);

		D_PHYSICS::ReleaseConvexMesh(mCurrentMeshUuid);
		mMeshIndicesReadback.Create(L"Convex Mesh Creation Indices Readback", indexBuffer.GetElementCount(), indexBuffer.GetElementSize());
		mCurrentMeshUuid = mReferenceMesh->GetUuid();

		context.CopyBuffer(mMeshIndicesReadback, indexBuffer);

		context.Finish(true);

		// Creating conv mesh
		{
			physx::PxConvexMeshDesc convDesc;
			convDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX | physx::PxConvexFlag::eGPU_COMPATIBLE | physx::PxConvexFlag::eFAST_INERTIA_COMPUTATION;
			convDesc.points.count = mMeshVerticesReadback.GetElementCount();
			convDesc.points.data = mMeshVerticesReadback.Map();
			convDesc.points.stride = mMeshVerticesReadback.GetElementSize();
			convDesc.indices.count = mMeshIndicesReadback.GetElementCount();
			convDesc.indices.data = mMeshIndicesReadback.Map();
			convDesc.indices.stride = mMeshIndicesReadback.GetElementSize();
			auto refUuid = mReferenceMesh->GetUuid();
			mMesh = D_PHYSICS::CreateConvexMesh(refUuid, false, convDesc);

#if _D_EDITOR
			mDebugMesh = D_PHYSICS::GetDebugMesh(refUuid);
#endif // _D_EDITOR

		}

		D_ASSERT(mMesh);

		mMeshVerticesReadback.Unmap();
		mMeshIndicesReadback.Unmap();

		mMeshVerticesReadback.Destroy();
		mMeshIndicesReadback.Destroy();

		CalculateScaledParameters();
		UpdateGeometry();
		InvalidatePhysicsActor();
	}

}
