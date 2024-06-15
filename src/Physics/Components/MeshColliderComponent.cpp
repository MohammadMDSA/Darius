#include "Physics/pch.hpp"
#include "MeshColliderComponent.hpp"

#include <Debug/DebugDraw.hpp>
#include <Renderer/Components/SkeletalMeshRendererComponent.hpp>
#include <Renderer/Components/MeshRendererComponent.hpp>

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
		mMesh(InvalidMeshDataHandle),
		mTightBounds(true),
		mDirectMesh(false),
		mConvex(false)
	{
		SetDirty();
	}

	MeshColliderComponent::MeshColliderComponent(D_CORE::Uuid const& uuid) :
		ColliderComponent(uuid),
		mMesh(InvalidMeshDataHandle),
		mTightBounds(true),
		mDirectMesh(false),
		mConvex(false)
	{
		SetDirty();
	}

	void MeshColliderComponent::Awake()
	{
		if(mReferenceMesh.IsValid())
		{
			CalculateMeshGeometry();
		}

		Super::Awake();
	}

#ifdef _D_EDITOR

	bool MeshColliderComponent::DrawDetails(float params[])
	{
		bool valueChanged = false;

		valueChanged |= Super::DrawDetails(params);

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Convex
		{
			bool value = IsConvex();
			D_H_DETAILS_DRAW_PROPERTY("Convex Mesh");
			if(ImGui::Checkbox("##Convex", &value))
			{
				SetConvex(value);
				valueChanged = true;
			}
		}

		// Tight bounds
		{
			bool value = IsTightBounds();
			D_H_DETAILS_DRAW_PROPERTY("Tight Bounds");
			if(ImGui::Checkbox("##TightBounds", &value))
			{
				SetTightBounds(value);
				valueChanged = true;
			}
		}

		// Direct Mesh
		{
			bool value = IsDirectMesh();
			D_H_DETAILS_DRAW_PROPERTY("Direct Mesh");
			if(ImGui::Checkbox("##DirectMesh", &value))
			{
				SetDirectMesh(value);
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
		if(!IsActive())
			return;

		if(!mMesh.IsValid())
			return;

		auto trans = GetTransform();
		auto rot = trans->GetRotation();
		auto offset = rot * GetScaledCenterOffset();
		D_DEBUG_DRAW::DrawMesh(trans->GetPosition() + offset, rot, trans->GetScale(), &mMesh->Mesh, 0., {0.f, 1.f, 0.f, 1.f});
	}

	void MeshColliderComponent::OnPostComponentAddInEditor()
	{
		Super::OnPostComponentAddInEditor();

		if(auto sm = GetGameObject()->GetComponent<D_RENDERER::MeshRendererComponent>())
		{
			auto mesh = sm->GetMesh();
			if(mesh && mesh->IsLoaded())
			{
				SetReferenceMesh(mesh);

			}
		}
	}

#endif // _D_EDITOR

	void MeshColliderComponent::OnDestroy()
	{

		mMesh = InvalidMeshDataHandle;

		Super::OnDestroy();
	}

	bool MeshColliderComponent::CalculateGeometry(physx::PxGeometry& geom) const
	{
		if(!mMesh.IsValid())
			return false;

		bool convex = IsConvex();

		// The convex state of the component must be consistent with the handle state
		D_ASSERT((convex && mMesh.Type == MeshType::ConvexMesh) ||
			!(convex && mMesh.Type == MeshType::TriangleMesh));

		PxMeshScale scale(D_PHYSICS::GetVec3(GetUsedScale()));

		if(convex)
		{
			PxConvexMeshGeometry& convMesh = reinterpret_cast<physx::PxConvexMeshGeometry&>(geom);

			PxConvexMeshGeometryFlags flags;
			if(IsTightBounds())
				flags |= PxConvexMeshGeometryFlag::eTIGHT_BOUNDS;

			convMesh = physx::PxConvexMeshGeometry(mMesh.ConvexMesh->PxMesh, scale, flags);
		}
		else
		{
			PxTriangleMeshGeometry& triMesh = reinterpret_cast<physx::PxTriangleMeshGeometry&>(geom);

			PxMeshGeometryFlags flags;
			if(IsTightBounds())
				flags |= PxMeshGeometryFlag::eTIGHT_BOUNDS;

			triMesh = physx::PxTriangleMeshGeometry(mMesh.TriangleMesh->PxMesh, scale, flags);
		}

		return true;
	}

	void MeshColliderComponent::SetTightBounds(bool tight)
	{
		if((bool)mTightBounds == tight)
			return;

		mTightBounds = tight;
		SetDirty();

		mChangeSignal(this);
	}

	void MeshColliderComponent::SetDirectMesh(bool direct)
	{
		if((bool)mDirectMesh == direct)
			return;

		mDirectMesh = direct;
		SetDirty();
		mChangeSignal(this);
	}

	void MeshColliderComponent::SetConvex(bool convex)
	{
		if((bool)mConvex == convex)
			return;

		mConvex = convex;

		CalculateMeshGeometry();

		SetDirty();
		mChangeSignal(this);
	}

	void MeshColliderComponent::SetReferenceMesh(D_RENDERER::StaticMeshResource* staticMesh)
	{

		if(mReferenceMesh == staticMesh)
			return;

		mReferenceMesh = staticMesh;

		auto meshValid = mReferenceMesh.IsValid();

		if(!meshValid || mReferenceMesh->IsLoaded())
		{
			// Mesh is already loaded
			CalculateMeshGeometry();
		}
		else if(meshValid)
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

	void MeshColliderComponent::CalculateConvexMeshGeometry()
	{
		mMesh = D_PHYSICS::FindConvexMesh(mReferenceMesh->GetUuid());
		if(mMesh.IsValid())
		{
			CalculateScaledParameters();
			UpdateGeometry();
			return;
		}

		auto& context = D_GRAPHICS::CommandContext::Begin(L"Convex Mesh Creation Mesh Readback");

		auto meshData = mReferenceMesh->GetMeshData();
		auto& vertBuffer = const_cast<D_GRAPHICS_BUFFERS::StructuredBuffer&>(meshData->VertexDataGpu);
		auto& indexBuffer = const_cast<D_GRAPHICS_BUFFERS::StructuredBuffer&>(meshData->IndexDataGpu[0]);

		mMeshVerticesReadback.Create(L"Convex Mesh Creation Vertices Readback", vertBuffer.GetElementCount(), vertBuffer.GetElementSize());
		context.CopyBuffer(mMeshVerticesReadback, vertBuffer);

		mMeshIndicesReadback.Create(L"Convex Mesh Creation Indices Readback", indexBuffer.GetElementCount(), indexBuffer.GetElementSize());
		mCurrentMeshUuid = mReferenceMesh->GetUuid();

		context.CopyBuffer(mMeshIndicesReadback, indexBuffer);

		context.Finish(true);

		// Creating conv mesh
		{
			physx::PxConvexMeshDesc convDesc;
			convDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX | physx::PxConvexFlag::eGPU_COMPATIBLE | physx::PxConvexFlag::eSHIFT_VERTICES;//  | physx::PxConvexFlag::eFAST_INERTIA_COMPUTATION;
			convDesc.points.count = mMeshVerticesReadback.GetElementCount();
			convDesc.points.data = mMeshVerticesReadback.Map();
			convDesc.points.stride = mMeshVerticesReadback.GetElementSize();
			convDesc.indices.count = mMeshIndicesReadback.GetElementCount();
			convDesc.indices.data = mMeshIndicesReadback.Map();
			convDesc.indices.stride = mMeshIndicesReadback.GetElementSize();
			auto refUuid = mReferenceMesh->GetUuid();

			mMesh = D_PHYSICS::CreateConvexMesh(refUuid, IsDirectMesh(), convDesc);
		}

		D_ASSERT(mMesh.ConvexMesh);

		mMeshVerticesReadback.Unmap();
		mMeshIndicesReadback.Unmap();

		mMeshVerticesReadback.Destroy();
		mMeshIndicesReadback.Destroy();

	}

	void MeshColliderComponent::CalculateTriangleMeshGeometry()
	{
		mMesh = D_PHYSICS::FindTriangleMesh(mReferenceMesh->GetUuid());
		if(mMesh.IsValid())
		{
			CalculateScaledParameters();
			UpdateGeometry();
			return;
		}

		auto& context = D_GRAPHICS::CommandContext::Begin(L"Triangle Mesh Creation Mesh Readback");

		auto meshData = mReferenceMesh->GetMeshData();
		auto& vertBuffer = const_cast<D_GRAPHICS_BUFFERS::StructuredBuffer&>(meshData->VertexDataGpu);
		auto& indexBuffer = const_cast<D_GRAPHICS_BUFFERS::StructuredBuffer&>(meshData->IndexDataGpu[0]);

		mMeshVerticesReadback.Create(L"Triangle Mesh Creation Vertices Readback", vertBuffer.GetElementCount(), vertBuffer.GetElementSize());
		context.CopyBuffer(mMeshVerticesReadback, vertBuffer);

		mMeshIndicesReadback.Create(L"Triangle Mesh Creation Indices Readback", indexBuffer.GetElementCount(), indexBuffer.GetElementSize());
		mCurrentMeshUuid = mReferenceMesh->GetUuid();

		context.CopyBuffer(mMeshIndicesReadback, indexBuffer);

		context.Finish(true);

		// Creating conv mesh
		{
			physx::PxTriangleMeshDesc triDesc;
			triDesc.points.count = mMeshVerticesReadback.GetElementCount();
			triDesc.points.data = mMeshVerticesReadback.Map();
			triDesc.points.stride = mMeshVerticesReadback.GetElementSize();

			UINT numIndices = mMeshIndicesReadback.GetElementCount();
			D_ASSERT(numIndices % 3 == 0);

			triDesc.triangles.count = numIndices / 3;
			triDesc.triangles.data = mMeshIndicesReadback.Map();
			triDesc.triangles.stride = mMeshIndicesReadback.GetElementSize() * 3;
			auto refUuid = mReferenceMesh->GetUuid();

			mMesh = D_PHYSICS::CreateTriangleMesh(refUuid, IsDirectMesh(), triDesc);
		}

		D_ASSERT(mMesh.ConvexMesh);

		mMeshVerticesReadback.Unmap();
		mMeshIndicesReadback.Unmap();

		mMeshVerticesReadback.Destroy();
		mMeshIndicesReadback.Destroy();

	}

	bool MeshColliderComponent::CalculateMeshGeometry()
	{
		if(!mReferenceMesh.IsValid())
		{
			mMesh = InvalidMeshDataHandle;
			return true;
		}

		D_ASSERT(mReferenceMesh.IsValid());

		if(!mReferenceMesh->IsLoaded())
		{
			mMesh = InvalidMeshDataHandle;
			return false;
		}

		if(IsConvex())
			CalculateConvexMeshGeometry();
		else
			CalculateTriangleMeshGeometry();

		CalculateScaledParameters();
		UpdateGeometry();

		return true;
	}

}
