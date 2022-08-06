#include "pch.hpp"
#include "MeshResource.hpp"

#include <Renderer/RenderDeviceManager.hpp>
#include <Renderer/GraphicsCore.hpp>

using namespace D_RENDERER_GEOMETRY;
using namespace D_CONTAINERS;

namespace Darius::ResourceManager
{
	void MeshResource::Create(std::wstring name, MeshData<VertexType>& data)
	{
		Destroy();

		this->mName = name;

		Mesh::Draw submesh;
		submesh.IndexCount = (UINT)data.Indices32.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		DVector<D_RENDERER_VERTEX::VertexPositionColor> vertices(data.Vertices.size());

		for (size_t i = 0; i < data.Vertices.size(); i++)
		{
			auto& ver = data.Vertices[i];
			vertices[i] = D_RENDERER_VERTEX::VertexPositionColor(ver.mPosition, (XMFLOAT4)DirectX::Colors::DarkGreen);
		}

		DVector<std::uint16_t> indices;
		indices.insert(indices.end(), std::begin(data.GetIndices16()), std::end(data.GetIndices16()));

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(D_RENDERER_VERTEX::VertexPositionColor);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		mMesh.name = name;

		D_HR_CHECK(D3DCreateBlob(vbByteSize, &mMesh.mVertexBufferCPU));
		CopyMemory(mMesh.mVertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		D_HR_CHECK(D3DCreateBlob(ibByteSize, &mMesh.mIndexBufferCPU));
		CopyMemory(mMesh.mIndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		auto& context = D_GRAPHICS::GraphicsContext::Begin(L"Mesh upload");

		mMesh.mVertexBufferGPU = D_RENDERER_UTILS::CreateDefaultBuffer(D_RENDERER_DEVICE::GetDevice(), context.GetCommandList(), vertices.data(), vbByteSize, mMesh.mVertexBufferUploader);

		mMesh.mIndexBufferGPU = D_RENDERER_UTILS::CreateDefaultBuffer(D_RENDERER_DEVICE::GetDevice(), context.GetCommandList(), indices.data(), ibByteSize, mMesh.mIndexBufferUploader);

		mMesh.mVertexByteStride = sizeof(D_RENDERER_VERTEX::VertexPositionColor);
		mMesh.mVertexBufferByteSize = vbByteSize;
		mMesh.mIndexFormat = DXGI_FORMAT_R16_UINT;
		mMesh.mIndexBufferByteSize = ibByteSize;

		mMesh.mDraw.push_back(submesh);
		context.Finish();

		mLoaded = true;
	}

}