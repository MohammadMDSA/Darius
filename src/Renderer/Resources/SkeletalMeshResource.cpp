#include "Renderer/pch.hpp"
#include "SkeletalMeshResource.hpp"

#include "Renderer/Geometry/ModelLoader/FbxLoader.hpp"

#ifdef _D_EDITOR
#include <imgui.h>
#endif

#include "SkeletalMeshResource.sgenerated.hpp"

using namespace D_RENDERER_GEOMETRY;

namespace Darius::Renderer
{

	D_CH_RESOURCE_DEF(SkeletalMeshResource);

	void SkeletalMeshResource::Create(MultiPartMeshData<VertexType> const& data)
	{
		Destroy();
		SetName(GetName());

		D_CONTAINERS::DVector<D_RENDERER_VERTEX::VertexPositionNormalTangentTextureSkinned> vertices;
		D_CONTAINERS::DVector<std::uint16_t> indices;

		// Filling vertex and index data
		for (int i = 0; i < data.MeshData.Vertices.size(); i++)
		{
			auto const& meshVertex = data.MeshData.Vertices[i];
			vertices.push_back(D_RENDERER_VERTEX::VertexPositionNormalTangentTextureSkinned(meshVertex.mPosition, D_MATH::Normalize(meshVertex.mNormal), meshVertex.mTangent, meshVertex.mTexC, meshVertex.mBlendIndices, meshVertex.mBlendWeights));
		}
		for (int i = 0; i < data.MeshData.Indices32.size(); i++)
		{
			indices.push_back(static_cast<std::uint16_t>(data.MeshData.Indices32[i]));
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

		mMesh.mBoundSp = mMesh.mBoundSp.Union(data.MeshData.CalcBoundingSphere());
		mMesh.mBoundBox = mMesh.mBoundBox.Union(data.MeshData.CalcBoundingBox());

		mMesh.Name = GetName();

		// Create vertex buffer
		mMesh.VertexDataGpu.Create(mMesh.Name + L" Vertex Buffer", (UINT)vertices.size(), sizeof(D_RENDERER_VERTEX::VertexPositionNormalTangentTextureSkinned), vertices.data());

		// Create index buffer
		mMesh.IndexDataGpu.Create(mMesh.Name + L" Index Buffer", (UINT)indices.size(), sizeof(std::uint16_t), indices.data());

		mJointCount = (UINT)mSkeleton.size();
		mSkeletonRoot = mJointCount > 0 ? &mSkeleton.front() : nullptr;
	}

	bool SkeletalMeshResource::UploadToGpu()
	{
		MultiPartMeshData<VertexType> meshData;

		D_RENDERER_GEOMETRY_LOADER_FBX::ReadMeshByName(GetPath(), GetName(), meshData, mSkeleton);

		Create(meshData);
		return true;
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
		DrawJoint(mSkeletonRoot);

		return false;
	}
#endif

}