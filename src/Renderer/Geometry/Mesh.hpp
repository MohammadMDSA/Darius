#pragma once

#include "Renderer/GraphicsUtils/Buffers/GpuBuffer.hpp"

#include <Math/Bounds/BoundingSphere.hpp>
#include <Math/Bounds/BoundingBox.hpp>
#include <Utils/Common.hpp>

#include <string>

#ifndef D_RENDERER_GEOMETRY
#define D_RENDERER_GEOMETRY Darius::Renderer::Geometry
#endif

using namespace Microsoft::WRL;
using namespace D_MATH;
using namespace Darius::Renderer::GraphicsUtils;

namespace Darius::Renderer::Geometry
{

	struct Mesh
	{

		struct Draw
		{
			UINT	IndexCount = 0;
			UINT	StartIndexLocation = 0;
			INT		BaseVertexLocation = 0;
		};

		struct SceneGraphNode
		{
			Math::Matrix4 xform;
			Math::Quaternion rotation;
			Math::XMFLOAT3 scale;
			std::string name;

			uint32_t matrixIdx : 28;
			uint32_t hasSibling : 1;
			uint32_t hasChildren : 1;
			uint32_t staleMatrix : 1;
			uint32_t skeletonRoot : 1;
		};

		INLINE D3D12_VERTEX_BUFFER_VIEW	VertexBufferView() const
		{
			return VertexDataGpu.VertexBufferView();
		}

		INLINE D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
		{
			return IndexDataGpu.IndexBufferView();
		}

		// Mesh name
		std::wstring name;

		D_GRAPHICS_BUFFERS::ByteAddressBuffer	VertexDataGpu;
		D_GRAPHICS_BUFFERS::ByteAddressBuffer	IndexDataGpu;

		// Submesh
		std::vector<Draw>				mDraw;

		D_MATH_BOUNDS::BoundingSphere	mBoundSp;
		D_MATH_BOUNDS::AxisAlignedBox	mBoundBox;

		UINT							mNumTotalVertices;
		UINT							mNumTotalIndices;
	};

}