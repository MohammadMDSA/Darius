#include "Renderer/pch.hpp"
#include "SkeletalMeshResource.hpp"

#include "Renderer/Geometry/ModelLoader/FbxLoader.hpp"

#include <imgui.h>

using namespace D_RENDERER_GEOMETRY;

namespace Darius::Graphics
{

	D_CH_RESOURCE_DEF(SkeletalMeshResource);

	void SkeletalMeshResource::Create(MultiPartMeshData<VertexType> const& data)
	{
		Destroy();
		SetName(GetName());

		auto totalVertices = 0u;
		auto totalIndices = 0u;
		for (auto meshData : data.meshParts)
		{
			totalVertices += meshData.Vertices.size();
			totalIndices += meshData.Indices32.size();
		}

		DVector<D_GRAPHICS_VERTEX::VertexPositionNormalTextureSkinned> vertices(totalVertices);
		DVector<std::uint16_t> indices;

		mMesh.mNumTotalIndices = totalIndices;
		mMesh.mNumTotalVertices = totalVertices;

		auto vertexIndex = 0;
		auto indexIndex = 0;
		for (auto meshData : data.MeshData)
		{
			// Creating submesh
			Mesh::Draw submesh;
			submesh.IndexCount = meshData.Indices32.size();
			submesh.StartIndexLocation = indexIndex;
			submesh.BaseVertexLocation = vertexIndex;

			// Adding vertices
			for (size_t i = 0; i < meshData.Vertices.size(); i++)
			{
				auto& ver = meshData.Vertices[i];
				vertices[vertexIndex] = D_GRAPHICS_VERTEX::VertexPositionNormalTextureSkinned(ver.mPosition, Vector3(ver.mNormal).Normalize(), ver.mTexC, ver.mBlendIndices, ver.mBlendWeights);
				vertexIndex++;
			}

			// Adding indices
			for (auto index : meshData.GetIndices16())
			{
				indices.push_back(index + submesh.BaseVertexLocation);
			}
			indexIndex += submesh.IndexCount;

			// Updating bounding sphear
			mMesh.mBoundSp = mMesh.mBoundSp.Union(meshData.CalcBoundingSphere());
			mMesh.mBoundBox = mMesh.mBoundBox.Union(meshData.CalcBoundingBox());

			mMesh.mDraw.push_back(submesh);
		}

		mMesh.Name = GetName();

		// Create vertex buffer
		mMesh.VertexDataGpu.Create(mMesh.Name + L" Vertex Buffer", vertices.size(), sizeof(D_GRAPHICS_VERTEX::VertexPositionNormalTextureSkinned), vertices.data());

		// Create index buffer
		mMesh.IndexDataGpu.Create(mMesh.Name + L" Index Buffer", indices.size(), sizeof(std::uint16_t), indices.data());

		mJointCount = mSkeleton.size();
		mSkeletonRoot = mJointCount > 0 ? &mSkeleton.front() : nullptr;
	}

	bool SkeletalMeshResource::UploadToGpu(void* ctx)
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