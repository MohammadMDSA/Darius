#pragma once

#include "Renderer/pch.hpp"


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
	interface IMeshData
	{
		virtual std::vector<uint16>& GetIndices16();
	};

	template<typename Vertex>
	struct MeshData : IMeshData
	{
		std::vector<Vertex>					mVertices;
		std::vector<uint32>					mIndices32;

		std::vector<uint16>& GetIndices16() override
		{
			if (mIndices16.empty())
			{
				mIndices16.resize(mIndices32.size());
				for (size_t i = 0; i < mIndices32.size(); i++)
					mIndices16[i] = static_cast<uint16>(mIndices32[i]);
			}

			return mIndices16;
		}

	private:
		std::vector<uint16>		mIndices16;
	};
}