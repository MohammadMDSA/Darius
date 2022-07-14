#pragma once

#include "GraphicsUtils/UploadBuffer.hpp"
#include "GraphicsUtils/D3DUtils.hpp"

#include <Utils/Common.hpp>

#include <string>

using namespace Microsoft::WRL;

using namespace Darius::Renderer::GraphicsUtils;

namespace Darius::Renderer
{

	struct Mesh
	{

		struct Draw
		{
			UINT	mIndexCount = 0;
			UINT	mStartIndexLocation = 0;
			INT		mBaseVertexLocation = 0;
		};

		INLINE D3D12_VERTEX_BUFFER_VIEW	VertexBufferView() const
		{
			D3D12_VERTEX_BUFFER_VIEW vbv;
			vbv.BufferLocation = mVertexBufferGPU->GetGPUVirtualAddress();
			vbv.SizeInBytes = mVertexBufferByteSize;
			vbv.StrideInBytes = mVertexByteStride;

			return vbv;
		}

		INLINE D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
		{
			D3D12_INDEX_BUFFER_VIEW ibv;
			ibv.BufferLocation = mIndexBufferGPU->GetGPUVirtualAddress();
			ibv.SizeInBytes = mIndexBufferByteSize;
			ibv.Format = mIndexFormat;

			return ibv;
		}

		INLINE void DisposeUploadBuffers()
		{
			mVertexBufferUploader = nullptr;
			mIndexBufferUploader = nullptr;
		}

		// Mesh name
		std::string name;

		// Main buffers on gpu
		ComPtr<ID3D12Resource>			mVertexBufferGPU = nullptr;
		ComPtr<ID3D12Resource>			mIndexBufferGPU = nullptr;

		// Main buffers on cpu
		ComPtr<ID3DBlob>				mVertexBufferCPU = nullptr;
		ComPtr<ID3DBlob>				mIndexBufferCPU = nullptr;

		// Upload buffers
		ComPtr<ID3D12Resource>			mVertexBufferUploader = nullptr;
		ComPtr<ID3D12Resource>			mIndexBufferUploader = nullptr;

		// Buffers info
		UINT							mVertexByteStride = 0;
		UINT							mVertexBufferByteSize = 0;
		DXGI_FORMAT						mIndexFormat = DXGI_FORMAT_R16_UINT;
		UINT							mIndexBufferByteSize = 0;

		// Submesh
		std::vector<Draw>				mDraw;
	};

}