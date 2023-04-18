#include "Renderer/pch.hpp"
#include "StaticMeshResource.hpp"

#include "Renderer/GraphicsDeviceManager.hpp"
#include "Renderer/GraphicsCore.hpp"

#include <Core/Filesystem/Path.hpp>
#include <Core/Containers/Set.hpp>
#include <ResourceManager/ResourceManager.hpp>

#define FBXSDK_SHARED

#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>

#include "StaticMeshResource.sgenerated.hpp"

using namespace D_CONTAINERS;
using namespace D_MATH;
using namespace D_RENDERER_GEOMETRY;

namespace Darius::Graphics
{

	D_CH_RESOURCE_DEF(StaticMeshResource);

	void StaticMeshResource::Create(D_RENDERER_GEOMETRY::MultiPartMeshData<VertexType> const& data)
	{
		Destroy();
		SetName(GetName());

		DVector<D_GRAPHICS_VERTEX::VertexPositionNormalTangentTexture> vertices;
		DVector<std::uint16_t> indices;

		// Filling vertex and index data
		for (int i = 0; i < data.MeshData.Vertices.size(); i++)
		{
			auto const& meshVertex = data.MeshData.Vertices[i];
			vertices.push_back(D_GRAPHICS_VERTEX::VertexPositionNormalTangentTexture(meshVertex.mPosition, D_MATH::Normalize(meshVertex.mNormal), meshVertex.mTangent, meshVertex.mTexC));
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

		mMesh.mBoundSp = mMesh.mBoundSp.Union(data.MeshData.CalcBoundingSphere());
		mMesh.mBoundBox = mMesh.mBoundBox.Union(data.MeshData.CalcBoundingBox());

		mMesh.Name = GetName();

		// Create vertex buffer
		mMesh.VertexDataGpu.Create(mMesh.Name + L" Vertex Buffer", (UINT)vertices.size(), sizeof(D_GRAPHICS_VERTEX::VertexPositionNormalTangentTexture), vertices.data());

		// Create index buffer
		mMesh.IndexDataGpu.Create(mMesh.Name + L" Index Buffer", (UINT)indices.size(), sizeof(std::uint16_t), indices.data());

	}

}