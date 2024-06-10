#pragma once

#include "Graphics/pch.hpp"

#include <Core/Containers/Vector.hpp>
#include <Math/Bounds/BoundingSphere.hpp>
#include <Math/Bounds/BoundingBox.hpp>


#ifndef D_RENDERER_GEOMETRY
#define D_RENDERER_GEOMETRY Darius::Renderer::Geometry
#endif

namespace
{
	using uint16 = std::uint16_t;
	using uint32 = std::uint32_t;
}

namespace Darius::Renderer::Geometry
{

	struct SubMesh
	{
		SubMesh() : IndexOffset(0), IndexCount(0) {}

		UINT IndexOffset;
		UINT IndexCount;
	};

	template<typename Vertex>
	struct MeshData
	{
		D_CONTAINERS::DVector<Vertex>	Vertices;
		D_CONTAINERS::DVector<uint32>	Indices32;

		D_CONTAINERS::DVector<uint16> const& GetIndices16()
		{
			if (mIndices16.empty())
			{
				mIndices16.resize(Indices32.size());
				for (size_t i = 0; i < Indices32.size(); i++)
					mIndices16[i] = static_cast<uint16>(Indices32[i]);
			}

			return mIndices16;
		}

		D_MATH_BOUNDS::BoundingSphere CalcBoundingSphere(D_MATH::Vector3 const& scale) const
		{
			if(scale.Equals(D_MATH::Vector3::One))
				return CalcBoundingSphere();

			D_MATH::Vector3 sumPos = D_MATH::Vector3::Zero;
			for(auto const& vert : Vertices)
			{
				sumPos += D_MATH::Vector3(vert.mPosition) * scale;
			}
			sumPos = sumPos / (float)Vertices.size();

			float radius = 0.f;
			for(auto const& vert : Vertices)
			{
				float distToCenter = D_MATH::Vector3::Distance(D_MATH::Vector3(vert.mPosition) * scale, sumPos);
				if(distToCenter > radius)
					radius = distToCenter;
			}

			return D_MATH_BOUNDS::BoundingSphere(sumPos, radius);
		}

		D_MATH_BOUNDS::BoundingSphere CalcBoundingSphere() const
		{
			D_MATH::Vector3 sumPos = D_MATH::Vector3::Zero;
			for(auto const& vert : Vertices)
			{
				sumPos += vert.mPosition;
			}
			sumPos = sumPos / (float)Vertices.size();

			float radius = 0.f;
			for(auto const& vert : Vertices)
			{
				float distToCenter = D_MATH::Vector3::Distance(vert.mPosition, sumPos);
				if(distToCenter > radius)
					radius = distToCenter;
			}

			return D_MATH_BOUNDS::BoundingSphere(sumPos, radius);
		}

		D_MATH_BOUNDS::AxisAlignedBox CalcBoundingBox(D_MATH::Vector3 const& scale) const
		{
			if(scale.Equals(D_MATH::Vector3::One))
				return CalcBoundingBox();

			D_MATH_BOUNDS::AxisAlignedBox box;
			bool first = true;
			for(auto const& vert : Vertices)
			{
				if(first)
				{
					first = false;
					D_MATH::Vector3 pos = D_MATH::Vector3(vert.mPosition) * scale;
					box = D_MATH_BOUNDS::AxisAlignedBox(pos, pos);
				}
				else
					box.AddPoint(D_MATH::Vector3(vert.mPosition) * scale);
			}

			return box;
		}

		D_MATH_BOUNDS::AxisAlignedBox CalcBoundingBox() const
		{
			D_MATH_BOUNDS::AxisAlignedBox box;
			bool first = true;
			for(auto const& vert : Vertices)
			{
				if(first)
				{
					first = false;
					box = D_MATH_BOUNDS::AxisAlignedBox(vert.mPosition, vert.mPosition);
				}
				else
					box.AddPoint(vert.mPosition);
			}

			return box;
		}

	private:
		D_CONTAINERS::DVector<uint16>	mIndices16;
	};

	template<typename Vertex>
	struct MultiPartMeshData
	{
		MeshData<Vertex>				MeshData;
		D_CONTAINERS::DVector<SubMesh>	SubMeshes;
	};

	// For each vertex, for each connected joint, index and (weight, init mat)
	typedef D_CONTAINERS::DVector<D_CONTAINERS::DVector<std::pair<UINT, std::pair<float, D_MATH::Matrix4>>>> VertexBlendWeightData;
}