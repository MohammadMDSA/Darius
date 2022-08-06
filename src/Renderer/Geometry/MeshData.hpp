#pragma once

#include "Renderer/pch.hpp"

#include <Core/Containers/Vector.hpp>


#ifndef D_RENDERER_GEOMETRY
#define D_RENDERER_GEOMETRY Darius::Renderer::Geometry
#endif

using namespace D_CONTAINERS;

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

	private:
		DVector<uint16>		mIndices16;
	};
}