#include "Renderer/pch.hpp"
#include "SkeletalMeshResource.hpp"

#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>
#include <Job/Job.hpp>

#ifdef _D_EDITOR
#include <imgui.h>
#endif

#include "SkeletalMeshResource.sgenerated.hpp"

using namespace D_RENDERER_GEOMETRY;

namespace Darius::Renderer
{

	D_CH_RESOURCE_DEF(SkeletalMeshResource);

	void SkeletalMeshResource::CreateInternal(MultiPartMeshData<VertexType> const& data)
	{
		Destroy();
		SetName(GetName());


		if (data.MeshData.Vertices.size() <= 0)
		{
			mMesh.Destroy();
			return;
		}

		D_CONTAINERS::DVector<D_RENDERER_VERTEX::VertexPositionNormalTangentTextureSkinned> vertices;
		D_CONTAINERS::DVector<std::uint32_t> indices;

		vertices.reserve(data.MeshData.Vertices.size());
		indices.reserve(data.MeshData.Indices32.size());

		// Filling vertex and index data
		for (int i = 0; i < data.MeshData.Vertices.size(); i++)
		{
			auto const& meshVertex = data.MeshData.Vertices[i];
			vertices.push_back(D_RENDERER_VERTEX::VertexPositionNormalTangentTextureSkinned(meshVertex.mPosition, D_MATH::Normalize(meshVertex.mNormal), meshVertex.mTangent, meshVertex.mTexC, meshVertex.mBlendIndices, meshVertex.mBlendWeights));
		}
		for (int i = 0; i < data.MeshData.Indices32.size(); i++)
		{
			indices.push_back(data.MeshData.Indices32[i]);
		}

		mMesh.mDraw.clear();
		if (data.SubMeshes.size() <= 0)
		{
			Mesh::Draw gpuDraw = { (UINT)indices.size(), 0, 0 };
			mMesh.mDraw.push_back(gpuDraw);
		}
		else
		{
			for (int i = 0; i < data.SubMeshes.size(); i++)
			{
				auto const& draw = data.SubMeshes[i];
				Mesh::Draw gpuDraw = { draw.IndexCount, draw.IndexOffset, 0 };
				mMesh.mDraw.push_back(gpuDraw);
			}
		}

		mMesh.mNumTotalVertices = (UINT)vertices.size();
		mMesh.mNumTotalIndices = (UINT)indices.size();

		mMesh.mBoundSp = data.MeshData.CalcBoundingSphere();
		auto fitAabb = data.MeshData.CalcBoundingBox();
		mMesh.mBoundBox = D_MATH_BOUNDS::Aabb::CreateFromCenterAndExtents(fitAabb.GetCenter(), fitAabb.GetExtents() + GetAabbExtentsBias());

		mMesh.Name = GetName();

		// Create vertex buffer
		mMesh.VertexDataGpu.Create(mMesh.Name + L" Vertex Buffer", (UINT)vertices.size(), sizeof(D_RENDERER_VERTEX::VertexPositionNormalTangentTextureSkinned), vertices.data());

		// Create index buffer
		mMesh.CreateIndexBuffers(indices.data());

		mJointCount = (UINT)mSkeleton.size();
		mSkeletonRoot = mJointCount > 0 ? &mSkeleton.front() : nullptr;


	}

	void SkeletalMeshResource::Create(D_RENDERER_GEOMETRY::MultiPartMeshData<VertexType> const& data, D_CONTAINERS::DList<D_RENDERER_GEOMETRY::Mesh::SkeletonJoint> const& skeleton)
	{

		D_CONTAINERS::DUnorderedMap<Mesh::SkeletonJoint const*, Mesh::SkeletonJoint*> equivalentMap;

		mSkeleton.clear();
		for (auto const& refJoint : skeleton)
		{
			Mesh::SkeletonJoint joint = refJoint;
			mSkeleton.push_back(joint);
			equivalentMap[&refJoint] = &mSkeleton.back();
		}
		for (auto& joint : mSkeleton)
		{
			for (int i = 0; i < (int)joint.Children.size(); i++)
			{
				joint.Children[i] = equivalentMap[joint.Children[i]];
			}
		}

		CreateInternal(data);
		SetMaterialListSize((UINT)data.SubMeshes.size());
	}

#ifdef _D_EDITOR
	void DrawJoint(const Mesh::SkeletonJoint* joint)
	{
		ImGuiTreeNodeFlags flag = joint->Children.size() ? 0 : ImGuiTreeNodeFlags_Leaf;
		if (ImGui::TreeNodeEx(joint->Name.c_str(), flag))
		{

			for (auto childNode : joint->Children)
			{
				DrawJoint(childNode);
			}

			ImGui::TreePop();
		}
	}

	bool SkeletalMeshResource::DrawDetails(float params[])
	{
		bool result = MeshResource::DrawDetails(params);

		DrawJoint(mSkeletonRoot);

		return result;
	}
#endif

}