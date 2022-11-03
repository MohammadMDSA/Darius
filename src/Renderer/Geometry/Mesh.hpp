#pragma once

#include "Renderer/GraphicsUtils/Buffers/GpuBuffer.hpp"

#include <Core/Containers/Vector.hpp>
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

		struct SkeletonJoint
		{
			Math::Matrix4			Xform;
			Math::XMFLOAT3			Rotation;
			Math::XMFLOAT3			Scale;
			std::string				Name;
			Matrix4					IBM;

			D_CONTAINERS::DVector<SkeletonJoint*> Children;

			uint32_t				MatrixIdx : 30;
			uint32_t				StaleMatrix : 1;
			uint32_t				SkeletonRoot : 1;
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
		std::wstring Name;

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