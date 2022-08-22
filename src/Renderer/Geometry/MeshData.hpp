#pragma once

#include "Renderer/pch.hpp"

#include <Core/Containers/Vector.hpp>
#include <Math/Bounds/BoundingSphere.hpp>
#include <Math/Bounds/BoundingBox.hpp>


#ifndef D_RENDERER_GEOMETRY
#define D_RENDERER_GEOMETRY Darius::Renderer::Geometry
#endif

using namespace D_CONTAINERS;
using namespace D_MATH_BOUNDS;

namespace
{
	using uint16 = std::uint16_t;
	using uint32 = std::uint32_t;
}

namespace Darius::Renderer::Geometry
{
	template<typename Vertex>
	struct MeshData
	{
		DVector<Vertex>					Vertices;
		DVector<uint32>					Indices32;

		DVector<uint16>& GetIndices16()
		{
			if (mIndices16.empty())
			{
				mIndices16.resize(Indices32.size());
				for (size_t i = 0; i < Indices32.size(); i++)
					mIndices16[i] = static_cast<uint16>(Indices32[i]);
			}

			return mIndices16;
		}

		BoundingSphere CalcBoundingSphere()
		{
			BoundingSphere res;
			for (auto vert : Vertices)
			{
				BoundingSphere vertBound(vert.mPosition, 0.001);
				res = res.Union(vertBound);
			}
			return res;
		}

		AxisAlignedBox CalcBoundingBox()
		{
			AxisAlignedBox box;
			for (auto vert : Vertices)
			{
				box.AddBoundingBox(AxisAlignedBox(vert.mPosition, vert.mPosition));
			}

			return box;
		}

	private:
		DVector<uint16>		mIndices16;
	};
}