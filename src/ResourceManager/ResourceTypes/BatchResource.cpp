#include "ResourceManager/pch.hpp"
#include "BatchResource.hpp"

#include <Renderer/GraphicsUtils/VertexTypes.hpp>
#include <Renderer/RenderDeviceManager.hpp>

namespace Darius::ResourceManager
{
	void BatchResource::Create(std::wstring name, MeshData<VertexType>& data)
	{
		Destroy();
		SetName(name);

		Mesh::Draw submesh;
		submesh.IndexCount = (UINT)data.Indices32.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		DVector<D_RENDERER_VERTEX::VertexPosition> vertices(data.Vertices.size());

		for (size_t i = 0; i < data.Vertices.size(); i++)
		{
			auto& ver = data.Vertices[i];
			vertices[i].mPosition = ver.mPosition;
		}

		DVector<std::uint16_t> indices;
		indices.insert(indices.end(), std::begin(data.GetIndices16()), std::end(data.GetIndices16()));

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(D_RENDERER_VERTEX::VertexPosition);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		mMesh.name = GetName();

		// Create vertex buffer
		mMesh.VertexDataGpu.Create(mMesh.name + L" Vertex Buffer", vertices.size(), sizeof(D_RENDERER_VERTEX::VertexPosition), vertices.data());

		// Create index buffer
		mMesh.IndexDataGpu.Create(mMesh.name + L" Index Buffer", indices.size(), sizeof(std::uint16_t), indices.data());

		mMesh.mDraw.push_back(submesh);

		mMesh.mBoundSp = data.CalcBoundingSphere();
	}

}
