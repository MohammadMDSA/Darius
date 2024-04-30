#include "Graphics/pch.hpp"
#include "Mesh.hpp"

#include <Graphics/CommandContext.hpp>

#include <sstream>

namespace Darius::Renderer::Geometry
{
	D_CORE::StringIdDatabase Mesh::SkeletonJoint::JointNameDatabase;

	Mesh::Mesh(Mesh const& other, std::wstring const& name, bool createVertexBuffer, bool perDrawIndexBuffer) :
		Name(name),
		mDraw(other.mDraw),
		mBoundSp(other.mBoundSp),
		mBoundBox(other.mBoundBox),
		mNumTotalIndices(other.mNumTotalIndices),
		mNumTotalVertices(other.mNumTotalVertices),
		mIndexBufferPerDraw(perDrawIndexBuffer)
	{
		if (createVertexBuffer)
		{
			VertexDataGpu.Create(name + L" Vertex Buffer", other.VertexDataGpu.GetElementCount(), other.VertexDataGpu.GetElementSize());
		}

		CreateIndexBuffers();
	}

	void Mesh::FillIndices(Mesh& other, class Darius::Graphics::CommandContext& context)
	{
		// Are the source mesh indices buffers per draw
		if (other.mIndexBufferPerDraw)
		{
			// Are the destination mesh indices buffers per draw
			if (mIndexBufferPerDraw)
			{
				D_ASSERT(IndexDataGpu.size() == mDraw.size());
				D_ASSERT(other.IndexDataGpu.size() == other.mDraw.size());

				for (int i = 0; i < IndexDataGpu.size(); i++)
				{
					context.TransitionResource(IndexDataGpu[i], D3D12_RESOURCE_STATE_COPY_DEST);
					context.TransitionResource(other.IndexDataGpu[i], D3D12_RESOURCE_STATE_COPY_SOURCE);
					context.CopyBuffer(IndexDataGpu[i], other.IndexDataGpu[i]);
				}

			}
			else
			{
				D_ASSERT(IndexDataGpu.size() == 1);
				D_ASSERT(other.IndexDataGpu.size() == other.mDraw.size());

				context.TransitionResource(IndexDataGpu[0], D3D12_RESOURCE_STATE_COPY_DEST);

				size_t accumulatedOffset = 0;

				for (int i = 0; i < IndexDataGpu.size(); i++)
				{
					context.TransitionResource(other.IndexDataGpu[i], D3D12_RESOURCE_STATE_COPY_SOURCE);
					size_t srcBufferSize = other.IndexDataGpu[i].GetBufferSize();
					context.CopyBufferRegion(IndexDataGpu[0], accumulatedOffset, other.IndexDataGpu[i], 0, srcBufferSize);
					accumulatedOffset += srcBufferSize;
				}
			}
		}
		else
		{
			// Are the destination mesh indices buffers per draw
			if (mIndexBufferPerDraw)
			{
				D_ASSERT(IndexDataGpu.size() == mDraw.size());
				D_ASSERT(other.IndexDataGpu.size() == 1);

				context.TransitionResource(other.IndexDataGpu[0], D3D12_RESOURCE_STATE_COPY_SOURCE);

				size_t accumulatedOffset = 0;

				for (int i = 0; i < IndexDataGpu.size(); i++)
				{
					context.TransitionResource(IndexDataGpu[i], D3D12_RESOURCE_STATE_COPY_DEST);
					size_t destBufferSize = IndexDataGpu[i].GetBufferSize();
					context.CopyBufferRegion(IndexDataGpu[i], 0, other.IndexDataGpu[0], accumulatedOffset, destBufferSize);
					accumulatedOffset += destBufferSize;
				}
			}
			else
			{
				D_ASSERT(IndexDataGpu.size() == 1);
				D_ASSERT(other.IndexDataGpu.size() == 1);

				context.TransitionResource(IndexDataGpu[0], D3D12_RESOURCE_STATE_COPY_DEST);
				context.TransitionResource(other.IndexDataGpu[0], D3D12_RESOURCE_STATE_COPY_SOURCE);
				context.CopyBuffer(IndexDataGpu[0], other.IndexDataGpu[0]);
			}
		}
	}

	void Mesh::CreateIndexBuffers(const void* initialData, D3D12_RESOURCE_STATES initialState)
	{
		if (mIndexBufferPerDraw)
		{
			IndexDataGpu.resize(mDraw.size());

			for (int i = 0; i < mDraw.size(); i++)
			{
				std::wstringstream nameStr;
				nameStr << Name << L" Index Buffer " << std::to_wstring(i);

				auto const& draw = mDraw.at(i);

				IndexDataGpu[i].Create(nameStr.str(), draw.IndexCount, (UINT)GetIndexElementSize(), initialData, initialState);
			}
		}
		else
		{
			IndexDataGpu.resize(1);
			IndexDataGpu[0].Create(Name + L" Index Buffer", mNumTotalIndices, (UINT)GetIndexElementSize(), initialData, initialState);
		}
	}
}
