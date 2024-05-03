#include "Renderer/pch.hpp"
#include "StaticMeshResource.hpp"


#include <Core/Filesystem/Path.hpp>
#include <Core/Containers/Set.hpp>
#include <Graphics/GraphicsDeviceManager.hpp>
#include <Graphics/GraphicsCore.hpp>
#include <ResourceManager/ResourceManager.hpp>

#define FBXSDK_SHARED
#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>

#include "StaticMeshResource.sgenerated.hpp"

using namespace D_CONTAINERS;
using namespace D_MATH;
using namespace D_RENDERER_GEOMETRY;

namespace Darius::Renderer
{

	D_CH_RESOURCE_DEF(StaticMeshResource);

	void StaticMeshResource::MakeVertexList(D_CONTAINERS::DVector<VertexType> const& inputVerts, D_CONTAINERS::DVector<D_RENDERER_VERTEX::VertexPositionNormalTangentTexture>& outVertices) const
	{
		Vector3 scale = GetScale();
		
		// For optimization purposes. If no scale, then process raw vec values
		if(scale.Equals(Vector3::One))
		{
			for(int i = 0; i < inputVerts.size(); i++)
			{
				auto const& meshVertex = inputVerts[i];
				outVertices.push_back(D_RENDERER_VERTEX::VertexPositionNormalTangentTexture(meshVertex.mPosition, D_MATH::Normalize(meshVertex.mNormal), meshVertex.mTangent, meshVertex.mTexC));
			}
			return;
		}

		// Dealing with uniform scale, no need to scale normals
		if(scale.GetX() == scale.GetY() && scale.GetY() == scale.GetZ())
		{
			for(int i = 0; i < inputVerts.size(); i++)
			{
				auto const& meshVertex = inputVerts[i];
				Vector3 scaledPos = Vector3(meshVertex.mPosition) * scale;
				outVertices.push_back(D_RENDERER_VERTEX::VertexPositionNormalTangentTexture(scaledPos, D_MATH::Normalize(meshVertex.mNormal), meshVertex.mTangent, meshVertex.mTexC));
			}
			return;
		}

		Vector3 invScale = D_MATH::Recip(scale);
		Vector4 invScale4 = Vector4(invScale, 1.f);

		for(int i = 0; i < inputVerts.size(); i++)
		{
			auto const& meshVertex = inputVerts[i];
			Vector3 pos = Vector3(meshVertex.mPosition) * scale;
			Vector3 normal = Vector3(meshVertex.mNormal) * invScale;
			Vector4 tangent = Vector4(meshVertex.mTangent) * invScale4;

			outVertices.push_back(D_RENDERER_VERTEX::VertexPositionNormalTangentTexture(pos, D_MATH::Normalize(normal), tangent, meshVertex.mTexC));
		}
	}

	void StaticMeshResource::CreateInternal(D_RENDERER_GEOMETRY::MultiPartMeshData<VertexType> const& data)
	{
		Destroy();
		SetName(GetName());

		if (data.MeshData.Vertices.size() <= 0)
		{
			mMesh.Destroy();
			return;
		}

		DVector<D_RENDERER_VERTEX::VertexPositionNormalTangentTexture> vertices;
		DVector<std::uint32_t> indices;

		vertices.reserve(data.MeshData.Vertices.size());
		indices.reserve(data.MeshData.Indices32.size());

		// Filling vertex and index data
		MakeVertexList(data.MeshData.Vertices, vertices);
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
		mMesh.VertexDataGpu.Create(mMesh.Name + L" Vertex Buffer", (UINT)vertices.size(), sizeof(D_RENDERER_VERTEX::VertexPositionNormalTangentTexture), vertices.data());

		// Create index buffer
		mMesh.CreateIndexBuffers(indices.data());
	}

	bool StaticMeshResource::DrawDetails(float params[])
	{
		return MeshResource::DrawDetails(params);
	}

}