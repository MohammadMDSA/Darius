//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#pragma once

#include "GraphicsUtils/GpuResource.hpp"
#include "GraphicsUtils/Buffers/ColorBuffer.hpp"
#include "GraphicsUtils/Buffers/PixelBuffer.hpp"
#include "GraphicsUtils/Buffers/GpuBuffer.hpp"
#include "GraphicsUtils/Buffers/UploadBuffer.hpp"
#include "GraphicsUtils/Buffers/DepthBuffer.hpp"
#include "GraphicsUtils/Buffers/ReadbackBuffer.hpp"
#include "GraphicsUtils/CommandListManager.hpp"
#include "GraphicsUtils/Memory/LinearAllocator.hpp"
#include "GraphicsUtils/Memory/DynamicDescriptorHeap.hpp"
#include "GraphicsUtils/PipelineState.hpp"
#include "GraphicsUtils/RootSignature.hpp"
#include "CommandSignature.hpp"
#include "GraphicsCore.hpp"

//#include "Texture.h"

#include <Utils/Assert.hpp>
#include <Utils/Common.hpp>
#include <Math/Color.hpp>
#include <Core/Memory/Memory.hpp>

#include <vector>

namespace Darius::Graphics
{
	class GraphicsContext;
	class ComputeContext;
}

namespace Darius::Graphics::Utils::Buffers
{
	class ColorBuffer;
	class DepthBuffer;
	class Texture;
	class UploadBuffer;
	class ReadbackBuffer;
}

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

namespace Darius::Graphics
{
	struct DWParam
	{
		DWParam(FLOAT f) : Float(f) {}
		DWParam(UINT u) : Uint(u) {}
		DWParam(INT i) : Int(i) {}

		void operator= (FLOAT f) { Float = f; }
		void operator= (UINT u) { Uint = u; }
		void operator= (INT i) { Int = i; }

		union
		{
			FLOAT Float;
			UINT Uint;
			INT Int;
		};
	};

#define VALID_COMPUTE_QUEUE_RESOURCE_STATES \
    ( D3D12_RESOURCE_STATE_UNORDERED_ACCESS \
    | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE \
    | D3D12_RESOURCE_STATE_COPY_DEST \
    | D3D12_RESOURCE_STATE_COPY_SOURCE )

	class ContextManager
	{
	public:
		ContextManager(void) {}

		CommandContext* AllocateContext(D3D12_COMMAND_LIST_TYPE Type);
		void FreeContext(CommandContext*);
		void DestroyAllContexts();

	private:
		std::vector<std::unique_ptr<CommandContext> > sm_ContextPool[4];
		std::queue<CommandContext*> sm_AvailableContexts[4];
		std::mutex sm_ContextAllocationMutex;
	};

	class CommandContext : NonCopyable
	{
		friend ContextManager;
	private:

		CommandContext(D3D12_COMMAND_LIST_TYPE Type);

		void Reset(void);

	public:

		~CommandContext(void);

		static void DestroyAllContexts(void);

		static CommandContext& Begin(const std::wstring ID = L"");

		// Flush existing commands to the GPU but keep the context alive
		uint64_t Flush(bool WaitForCompletion = false);

		// Flush existing commands and release the current context
		uint64_t Finish(bool WaitForCompletion = false);

		// Prepare to render by reserving a command list and command allocator
		void Initialize(void);

		GraphicsContext& GetGraphicsContext() {
			D_ASSERT_M(m_Type != D3D12_COMMAND_LIST_TYPE_COMPUTE, "Cannot convert async compute context to graphics");
			return reinterpret_cast<GraphicsContext&>(*this);
		}

		ComputeContext& GetComputeContext() {
			return reinterpret_cast<ComputeContext&>(*this);
		}

		ID3D12GraphicsCommandList* GetCommandList() {
			return m_CommandList;
		}

		void CopyBuffer(D_GRAPHICS_UTILS::GpuResource& Dest, D_GRAPHICS_UTILS::GpuResource& Src);
		void CopyBufferRegion(D_GRAPHICS_UTILS::GpuResource& Dest, size_t DestOffset, D_GRAPHICS_UTILS::GpuResource& Src, size_t SrcOffset, size_t NumBytes);
		void CopySubresource(D_GRAPHICS_UTILS::GpuResource& Dest, UINT DestSubIndex, D_GRAPHICS_UTILS::GpuResource& Src, UINT SrcSubIndex);
		void CopyCounter(D_GRAPHICS_UTILS::GpuResource& Dest, size_t DestOffset, D_GRAPHICS_BUFFERS::StructuredBuffer& Src);
		void CopyTextureRegion(D_GRAPHICS_UTILS::GpuResource& Dest, UINT x, UINT y, UINT z, D_GRAPHICS_UTILS::GpuResource& Source, RECT& rect);
		void ResetCounter(D_GRAPHICS_BUFFERS::StructuredBuffer& Buf, uint32_t Value = 0);

		// Creates a readback buffer of sufficient size, copies the texture into it,
		// and returns row pitch in bytes.
		uint32_t ReadbackTexture(D_GRAPHICS_BUFFERS::ReadbackBuffer& DstBuffer, D_GRAPHICS_BUFFERS::PixelBuffer& SrcBuffer);

		D_GRAPHICS_MEMORY::DynAlloc ReserveUploadMemory(size_t SizeInBytes)
		{
			return m_CpuLinearAllocator.Allocate(SizeInBytes);
		}

		static void InitializeTexture(D_GRAPHICS_UTILS::GpuResource& Dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA SubData[]);
		static void InitializeBuffer(D_GRAPHICS_BUFFERS::GpuBuffer& Dest, const void* Data, size_t NumBytes, size_t DestOffset = 0);
		static void InitializeBuffer(D_GRAPHICS_BUFFERS::GpuBuffer& Dest, const D_GRAPHICS_BUFFERS::UploadBuffer& Src, size_t SrcOffset, size_t NumBytes = -1, size_t DestOffset = 0);
		static void InitializeTextureArraySlice(D_GRAPHICS_UTILS::GpuResource& Dest, UINT SliceIndex, D_GRAPHICS_UTILS::GpuResource& Src);

		void WriteBuffer(D_GRAPHICS_UTILS::GpuResource& Dest, size_t DestOffset, const void* Data, size_t NumBytes);
		void FillBuffer(D_GRAPHICS_UTILS::GpuResource& Dest, size_t DestOffset, DWParam Value, size_t NumBytes);

		void TransitionResource(D_GRAPHICS_UTILS::GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
		void BeginResourceTransition(D_GRAPHICS_UTILS::GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
		void InsertUAVBarrier(D_GRAPHICS_UTILS::GpuResource& Resource, bool FlushImmediate = false);
		void InsertAliasBarrier(D_GRAPHICS_UTILS::GpuResource& Before, D_GRAPHICS_UTILS::GpuResource& After, bool FlushImmediate = false);
		inline void FlushResourceBarriers(void);

		void InsertTimeStamp(ID3D12QueryHeap* pQueryHeap, uint32_t QueryIdx);
		void ResolveTimeStamps(ID3D12Resource* pReadbackHeap, ID3D12QueryHeap* pQueryHeap, uint32_t NumQueries);
		void PIXBeginEvent(const wchar_t* label);
		void PIXEndEvent(void);
		void PIXSetMarker(const wchar_t* label);

		void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr);
		void SetDescriptorHeaps(UINT HeapCount, D3D12_DESCRIPTOR_HEAP_TYPE Type[], ID3D12DescriptorHeap* HeapPtrs[]);
		void SetPipelineState(const D_GRAPHICS_UTILS::PSO& PSO);

		void SetPredication(ID3D12Resource* Buffer, UINT64 BufferOffset, D3D12_PREDICATION_OP Op);

	protected:

		void BindDescriptorHeaps(void);

		D_GRAPHICS_UTILS::CommandListManager* m_OwningManager;
		ID3D12GraphicsCommandList* m_CommandList;
		ID3D12CommandAllocator* m_CurrentAllocator;

		ID3D12RootSignature* m_CurGraphicsRootSignature;
		ID3D12RootSignature* m_CurComputeRootSignature;
		ID3D12PipelineState* m_CurPipelineState;

		D_GRAPHICS_MEMORY::DynamicDescriptorHeap m_DynamicViewDescriptorHeap;		// HEAP_TYPE_CBV_SRV_UAV
		D_GRAPHICS_MEMORY::DynamicDescriptorHeap m_DynamicSamplerDescriptorHeap;	// HEAP_TYPE_SAMPLER

		D3D12_RESOURCE_BARRIER m_ResourceBarrierBuffer[16];
		UINT m_NumBarriersToFlush;

		ID3D12DescriptorHeap* m_CurrentDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

		D_GRAPHICS_MEMORY::LinearAllocator m_CpuLinearAllocator;
		D_GRAPHICS_MEMORY::LinearAllocator m_GpuLinearAllocator;

		std::wstring m_ID;
		void SetID(const std::wstring& ID) { m_ID = ID; }

		D3D12_COMMAND_LIST_TYPE m_Type;
	};

	class GraphicsContext : public CommandContext
	{
	public:

		static GraphicsContext& Begin(const std::wstring& ID = L"")
		{
			return CommandContext::Begin(ID).GetGraphicsContext();
		}

		void ClearUAV(D_GRAPHICS_BUFFERS::GpuBuffer& Target);
		void ClearUAV(D_GRAPHICS_BUFFERS::ColorBuffer& Target);
		void ClearColor(D_GRAPHICS_BUFFERS::ColorBuffer& Target, D3D12_RECT* Rect = nullptr);
		void ClearColor(D_GRAPHICS_BUFFERS::ColorBuffer& Target, float Colour[4], D3D12_RECT* Rect = nullptr);
		void ClearDepth(D_GRAPHICS_BUFFERS::DepthBuffer& Target);
		void ClearStencil(D_GRAPHICS_BUFFERS::DepthBuffer& Target);
		void ClearDepthAndStencil(D_GRAPHICS_BUFFERS::DepthBuffer& Target);

		void BeginQuery(ID3D12QueryHeap* QueryHeap, D3D12_QUERY_TYPE Type, UINT HeapIndex);
		void EndQuery(ID3D12QueryHeap* QueryHeap, D3D12_QUERY_TYPE Type, UINT HeapIndex);
		void ResolveQueryData(ID3D12QueryHeap* QueryHeap, D3D12_QUERY_TYPE Type, UINT StartIndex, UINT NumQueries, ID3D12Resource* DestinationBuffer, UINT64 DestinationBufferOffset);

		void SetRootSignature(const D_GRAPHICS_UTILS::RootSignature& RootSig);

		void SetRenderTargets(UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[]);
		void SetRenderTargets(UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[], D3D12_CPU_DESCRIPTOR_HANDLE DSV);
		void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE RTV) { SetRenderTargets(1, &RTV); }
		void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE RTV, D3D12_CPU_DESCRIPTOR_HANDLE DSV) { SetRenderTargets(1, &RTV, DSV); }
		void SetDepthStencilTarget(D3D12_CPU_DESCRIPTOR_HANDLE DSV) { SetRenderTargets(0, nullptr, DSV); }

		void SetViewport(const D3D12_VIEWPORT& vp);
		void SetViewport(FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT minDepth = 0.0f, FLOAT maxDepth = 1.0f);
		void SetScissor(const D3D12_RECT& rect);
		void SetScissor(UINT left, UINT top, UINT right, UINT bottom);
		void SetViewportAndScissor(const D3D12_VIEWPORT& vp, const D3D12_RECT& rect);
		void SetViewportAndScissor(UINT x, UINT y, UINT w, UINT h);
		void SetStencilRef(UINT StencilRef);
		void SetBlendFactor(D_MATH::Color BlendFactor);
		void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology);

		void SetConstantArray(UINT RootIndex, UINT NumConstants, const void* pConstants);
		void SetConstant(UINT RootIndex, UINT Offset, DWParam Val);
		void SetConstants(UINT RootIndex, DWParam X);
		void SetConstants(UINT RootIndex, DWParam X, DWParam Y);
		void SetConstants(UINT RootIndex, DWParam X, DWParam Y, DWParam Z);
		void SetConstants(UINT RootIndex, DWParam X, DWParam Y, DWParam Z, DWParam W);
		void SetConstantBuffer(UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS CBV);
		void SetDynamicConstantBufferView(UINT RootIndex, size_t BufferSize, const void* BufferData);
		void SetBufferSRV(UINT RootIndex, const D_GRAPHICS_BUFFERS::GpuBuffer& SRV, UINT64 Offset = 0);
		void SetBufferUAV(UINT RootIndex, const D_GRAPHICS_BUFFERS::GpuBuffer& UAV, UINT64 Offset = 0);
		void SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle);

		void SetDynamicDescriptor(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle);
		void SetDynamicDescriptors(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]);
		void SetDynamicSampler(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle);
		void SetDynamicSamplers(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]);

		void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& IBView);
		void SetVertexBuffer(UINT Slot, const D3D12_VERTEX_BUFFER_VIEW& VBView);
		void SetVertexBuffers(UINT StartSlot, UINT Count, const D3D12_VERTEX_BUFFER_VIEW VBViews[]);
		void SetDynamicVB(UINT Slot, size_t NumVertices, size_t VertexStride, const void* VBData);
		void SetDynamicIB(size_t IndexCount, const uint16_t* IBData);
		void SetDynamicSRV(UINT RootIndex, size_t BufferSize, const void* BufferData);

		void Draw(UINT VertexCount, UINT VertexStartOffset = 0);
		void DrawIndexed(UINT IndexCount, UINT StartIndexLocation = 0, INT BaseVertexLocation = 0);
		void DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount,
			UINT StartVertexLocation = 0, UINT StartInstanceLocation = 0);
		void DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation,
			INT BaseVertexLocation, UINT StartInstanceLocation);
		void DrawIndirect(D_GRAPHICS_BUFFERS::GpuBuffer& ArgumentBuffer, uint64_t ArgumentBufferOffset = 0);
		void ExecuteIndirect(CommandSignature& CommandSig, D_GRAPHICS_BUFFERS::GpuBuffer& ArgumentBuffer, uint64_t ArgumentStartOffset = 0,
			uint32_t MaxCommands = 1, D_GRAPHICS_BUFFERS::GpuBuffer* CommandCounterBuffer = nullptr, uint64_t CounterOffset = 0);

	private:
	};

	class ComputeContext : public CommandContext
	{
	public:

		static ComputeContext& Begin(const std::wstring& ID = L"", bool Async = false);

		void ClearUAV(D_GRAPHICS_BUFFERS::GpuBuffer& Target);
		void ClearUAV(D_GRAPHICS_BUFFERS::ColorBuffer& Target);

		void SetRootSignature(const D_GRAPHICS_UTILS::RootSignature& RootSig);

		void SetConstantArray(UINT RootIndex, UINT NumConstants, const void* pConstants);
		void SetConstant(UINT RootIndex, UINT Offset, DWParam Val);
		void SetConstants(UINT RootIndex, DWParam X);
		void SetConstants(UINT RootIndex, DWParam X, DWParam Y);
		void SetConstants(UINT RootIndex, DWParam X, DWParam Y, DWParam Z);
		void SetConstants(UINT RootIndex, DWParam X, DWParam Y, DWParam Z, DWParam W);
		void SetConstantBuffer(UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS CBV);
		void SetDynamicConstantBufferView(UINT RootIndex, size_t BufferSize, const void* BufferData);
		void SetDynamicSRV(UINT RootIndex, size_t BufferSize, const void* BufferData);
		void SetBufferSRV(UINT RootIndex, const D_GRAPHICS_BUFFERS::GpuBuffer& SRV, UINT64 Offset = 0);
		void SetBufferUAV(UINT RootIndex, const D_GRAPHICS_BUFFERS::GpuBuffer& UAV, UINT64 Offset = 0);
		void SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle);

		void SetDynamicDescriptor(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle);
		void SetDynamicDescriptors(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]);
		void SetDynamicSampler(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle);
		void SetDynamicSamplers(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]);

		void Dispatch(size_t GroupCountX = 1, size_t GroupCountY = 1, size_t GroupCountZ = 1);
		void Dispatch1D(size_t ThreadCountX, size_t GroupSizeX = 64);
		void Dispatch2D(size_t ThreadCountX, size_t ThreadCountY, size_t GroupSizeX = 8, size_t GroupSizeY = 8);
		void Dispatch3D(size_t ThreadCountX, size_t ThreadCountY, size_t ThreadCountZ, size_t GroupSizeX, size_t GroupSizeY, size_t GroupSizeZ);
		void DispatchIndirect(D_GRAPHICS_BUFFERS::GpuBuffer& ArgumentBuffer, uint64_t ArgumentBufferOffset = 0);
		void ExecuteIndirect(CommandSignature& CommandSig, D_GRAPHICS_BUFFERS::GpuBuffer& ArgumentBuffer, uint64_t ArgumentStartOffset = 0,
			uint32_t MaxCommands = 1, D_GRAPHICS_BUFFERS::GpuBuffer* CommandCounterBuffer = nullptr, uint64_t CounterOffset = 0);

	private:
	};

	inline void CommandContext::FlushResourceBarriers(void)
	{
		if (m_NumBarriersToFlush > 0)
		{
			m_CommandList->ResourceBarrier(m_NumBarriersToFlush, m_ResourceBarrierBuffer);
			m_NumBarriersToFlush = 0;
		}
	}

	inline void GraphicsContext::SetRootSignature(const D_GRAPHICS_UTILS::RootSignature& RootSig)
	{
		if (RootSig.GetSignature() == m_CurGraphicsRootSignature)
			return;

		m_CommandList->SetGraphicsRootSignature(m_CurGraphicsRootSignature = RootSig.GetSignature());

		m_DynamicViewDescriptorHeap.ParseGraphicsRootSignature(RootSig);
		m_DynamicSamplerDescriptorHeap.ParseGraphicsRootSignature(RootSig);
	}

	inline void ComputeContext::SetRootSignature(const D_GRAPHICS_UTILS::RootSignature& RootSig)
	{
		if (RootSig.GetSignature() == m_CurComputeRootSignature)
			return;

		m_CommandList->SetComputeRootSignature(m_CurComputeRootSignature = RootSig.GetSignature());

		m_DynamicViewDescriptorHeap.ParseComputeRootSignature(RootSig);
		m_DynamicSamplerDescriptorHeap.ParseComputeRootSignature(RootSig);
	}

	inline void CommandContext::SetPipelineState(const D_GRAPHICS_UTILS::PSO& PSO)
	{
		ID3D12PipelineState* PipelineState = PSO.GetPipelineStateObject();
		if (PipelineState == m_CurPipelineState)
			return;

		m_CommandList->SetPipelineState(PipelineState);
		m_CurPipelineState = PipelineState;
	}

	inline void GraphicsContext::SetViewportAndScissor(UINT x, UINT y, UINT w, UINT h)
	{
		SetViewport((float)x, (float)y, (float)w, (float)h);
		SetScissor(x, y, x + w, y + h);
	}

	inline void GraphicsContext::SetScissor(UINT left, UINT top, UINT right, UINT bottom)
	{
		SetScissor(CD3DX12_RECT(left, top, right, bottom));
	}

	inline void GraphicsContext::SetStencilRef(UINT ref)
	{
		m_CommandList->OMSetStencilRef(ref);
	}

	inline void GraphicsContext::SetBlendFactor(D_MATH::Color BlendFactor)
	{
		m_CommandList->OMSetBlendFactor(BlendFactor.GetPtr());
	}

	inline void GraphicsContext::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology)
	{
		m_CommandList->IASetPrimitiveTopology(Topology);
	}

	inline void ComputeContext::SetConstantArray(UINT RootEntry, UINT NumConstants, const void* pConstants)
	{
		m_CommandList->SetComputeRoot32BitConstants(RootEntry, NumConstants, pConstants, 0);
	}

	inline void ComputeContext::SetConstant(UINT RootEntry, UINT Offset, DWParam Val)
	{
		m_CommandList->SetComputeRoot32BitConstant(RootEntry, Val.Uint, Offset);
	}

	inline void ComputeContext::SetConstants(UINT RootEntry, DWParam X)
	{
		m_CommandList->SetComputeRoot32BitConstant(RootEntry, X.Uint, 0);
	}

	inline void ComputeContext::SetConstants(UINT RootEntry, DWParam X, DWParam Y)
	{
		m_CommandList->SetComputeRoot32BitConstant(RootEntry, X.Uint, 0);
		m_CommandList->SetComputeRoot32BitConstant(RootEntry, Y.Uint, 1);
	}

	inline void ComputeContext::SetConstants(UINT RootEntry, DWParam X, DWParam Y, DWParam Z)
	{
		m_CommandList->SetComputeRoot32BitConstant(RootEntry, X.Uint, 0);
		m_CommandList->SetComputeRoot32BitConstant(RootEntry, Y.Uint, 1);
		m_CommandList->SetComputeRoot32BitConstant(RootEntry, Z.Uint, 2);
	}

	inline void ComputeContext::SetConstants(UINT RootEntry, DWParam X, DWParam Y, DWParam Z, DWParam W)
	{
		m_CommandList->SetComputeRoot32BitConstant(RootEntry, X.Uint, 0);
		m_CommandList->SetComputeRoot32BitConstant(RootEntry, Y.Uint, 1);
		m_CommandList->SetComputeRoot32BitConstant(RootEntry, Z.Uint, 2);
		m_CommandList->SetComputeRoot32BitConstant(RootEntry, W.Uint, 3);
	}

	inline void GraphicsContext::SetConstantArray(UINT RootIndex, UINT NumConstants, const void* pConstants)
	{
		m_CommandList->SetGraphicsRoot32BitConstants(RootIndex, NumConstants, pConstants, 0);
	}

	inline void GraphicsContext::SetConstant(UINT RootEntry, UINT Offset, DWParam Val)
	{
		m_CommandList->SetGraphicsRoot32BitConstant(RootEntry, Val.Uint, Offset);
	}

	inline void GraphicsContext::SetConstants(UINT RootIndex, DWParam X)
	{
		m_CommandList->SetGraphicsRoot32BitConstant(RootIndex, X.Uint, 0);
	}

	inline void GraphicsContext::SetConstants(UINT RootIndex, DWParam X, DWParam Y)
	{
		m_CommandList->SetGraphicsRoot32BitConstant(RootIndex, X.Uint, 0);
		m_CommandList->SetGraphicsRoot32BitConstant(RootIndex, Y.Uint, 1);
	}

	inline void GraphicsContext::SetConstants(UINT RootIndex, DWParam X, DWParam Y, DWParam Z)
	{
		m_CommandList->SetGraphicsRoot32BitConstant(RootIndex, X.Uint, 0);
		m_CommandList->SetGraphicsRoot32BitConstant(RootIndex, Y.Uint, 1);
		m_CommandList->SetGraphicsRoot32BitConstant(RootIndex, Z.Uint, 2);
	}

	inline void GraphicsContext::SetConstants(UINT RootIndex, DWParam X, DWParam Y, DWParam Z, DWParam W)
	{
		m_CommandList->SetGraphicsRoot32BitConstant(RootIndex, X.Uint, 0);
		m_CommandList->SetGraphicsRoot32BitConstant(RootIndex, Y.Uint, 1);
		m_CommandList->SetGraphicsRoot32BitConstant(RootIndex, Z.Uint, 2);
		m_CommandList->SetGraphicsRoot32BitConstant(RootIndex, W.Uint, 3);
	}

	inline void ComputeContext::SetConstantBuffer(UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS CBV)
	{
		m_CommandList->SetComputeRootConstantBufferView(RootIndex, CBV);
	}

	inline void GraphicsContext::SetConstantBuffer(UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS CBV)
	{
		m_CommandList->SetGraphicsRootConstantBufferView(RootIndex, CBV);
	}

	inline void GraphicsContext::SetDynamicConstantBufferView(UINT RootIndex, size_t BufferSize, const void* BufferData)
	{
		D_ASSERT(BufferData != nullptr && D_MEMORY::IsAligned(BufferData, 16));
		D_GRAPHICS_MEMORY::DynAlloc cb = m_CpuLinearAllocator.Allocate(BufferSize);
		//SIMDMemCopy(cb.DataPtr, BufferData, Math::AlignUp(BufferSize, 16) >> 4);
		memcpy(cb.DataPtr, BufferData, BufferSize);
		m_CommandList->SetGraphicsRootConstantBufferView(RootIndex, cb.GpuAddress);
	}

	inline void ComputeContext::SetDynamicConstantBufferView(UINT RootIndex, size_t BufferSize, const void* BufferData)
	{
		D_ASSERT(BufferData != nullptr && D_MEMORY::IsAligned(BufferData, 16));
		D_GRAPHICS_MEMORY::DynAlloc cb = m_CpuLinearAllocator.Allocate(BufferSize);
		//SIMDMemCopy(cb.DataPtr, BufferData, Math::AlignUp(BufferSize, 16) >> 4);
		memcpy(cb.DataPtr, BufferData, BufferSize);
		m_CommandList->SetComputeRootConstantBufferView(RootIndex, cb.GpuAddress);
	}

	inline void GraphicsContext::SetDynamicVB(UINT Slot, size_t NumVertices, size_t VertexStride, const void* VertexData)
	{
		D_ASSERT(VertexData != nullptr && D_MEMORY::IsAligned(VertexData, 16));

		size_t BufferSize = D_MEMORY::AlignUp(NumVertices * VertexStride, 16);
		D_GRAPHICS_MEMORY::DynAlloc vb = m_CpuLinearAllocator.Allocate(BufferSize);

		D_MEMORY::SIMDMemCopy(vb.DataPtr, VertexData, BufferSize >> 4);

		D3D12_VERTEX_BUFFER_VIEW VBView;
		VBView.BufferLocation = vb.GpuAddress;
		VBView.SizeInBytes = (UINT)BufferSize;
		VBView.StrideInBytes = (UINT)VertexStride;

		m_CommandList->IASetVertexBuffers(Slot, 1, &VBView);
	}

	inline void GraphicsContext::SetDynamicIB(size_t IndexCount, const uint16_t* IndexData)
	{
		D_ASSERT(IndexData != nullptr && D_MEMORY::IsAligned(IndexData, 16));

		size_t BufferSize = D_MEMORY::AlignUp(IndexCount * sizeof(uint16_t), 16);
		D_GRAPHICS_MEMORY::DynAlloc ib = m_CpuLinearAllocator.Allocate(BufferSize);

		D_MEMORY::SIMDMemCopy(ib.DataPtr, IndexData, BufferSize >> 4);

		D3D12_INDEX_BUFFER_VIEW IBView;
		IBView.BufferLocation = ib.GpuAddress;
		IBView.SizeInBytes = (UINT)(IndexCount * sizeof(uint16_t));
		IBView.Format = DXGI_FORMAT_R16_UINT;

		m_CommandList->IASetIndexBuffer(&IBView);
	}

	inline void GraphicsContext::SetDynamicSRV(UINT RootIndex, size_t BufferSize, const void* BufferData)
	{
		D_ASSERT(BufferData != nullptr && D_MEMORY::IsAligned(BufferData, 16));
		D_GRAPHICS_MEMORY::DynAlloc cb = m_CpuLinearAllocator.Allocate(BufferSize);
		D_MEMORY::SIMDMemCopy(cb.DataPtr, BufferData, D_MEMORY::AlignUp(BufferSize, 16) >> 4);
		m_CommandList->SetGraphicsRootShaderResourceView(RootIndex, cb.GpuAddress);
	}

	inline void ComputeContext::SetDynamicSRV(UINT RootIndex, size_t BufferSize, const void* BufferData)
	{
		D_ASSERT(BufferData != nullptr && D_MEMORY::IsAligned(BufferData, 16));
		D_GRAPHICS_MEMORY::DynAlloc cb = m_CpuLinearAllocator.Allocate(BufferSize);
		D_MEMORY::SIMDMemCopy(cb.DataPtr, BufferData, D_MEMORY::AlignUp(BufferSize, 16) >> 4);
		m_CommandList->SetComputeRootShaderResourceView(RootIndex, cb.GpuAddress);
	}

	inline void GraphicsContext::SetBufferSRV(UINT RootIndex, const D_GRAPHICS_BUFFERS::GpuBuffer& SRV, UINT64 Offset)
	{
		D_ASSERT((SRV.mUsageState & (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)) != 0);
		m_CommandList->SetGraphicsRootShaderResourceView(RootIndex, SRV.GetGpuVirtualAddress() + Offset);
	}

	inline void ComputeContext::SetBufferSRV(UINT RootIndex, const D_GRAPHICS_BUFFERS::GpuBuffer& SRV, UINT64 Offset)
	{
		D_ASSERT((SRV.mUsageState & D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE) != 0);
		m_CommandList->SetComputeRootShaderResourceView(RootIndex, SRV.GetGpuVirtualAddress() + Offset);
	}

	inline void GraphicsContext::SetBufferUAV(UINT RootIndex, const D_GRAPHICS_BUFFERS::GpuBuffer& UAV, UINT64 Offset)
	{
		D_ASSERT((UAV.mUsageState & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) != 0);
		m_CommandList->SetGraphicsRootUnorderedAccessView(RootIndex, UAV.GetGpuVirtualAddress() + Offset);
	}

	inline void ComputeContext::SetBufferUAV(UINT RootIndex, const D_GRAPHICS_BUFFERS::GpuBuffer& UAV, UINT64 Offset)
	{
		D_ASSERT((UAV.mUsageState & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) != 0);
		m_CommandList->SetComputeRootUnorderedAccessView(RootIndex, UAV.GetGpuVirtualAddress() + Offset);
	}

	inline void ComputeContext::Dispatch(size_t GroupCountX, size_t GroupCountY, size_t GroupCountZ)
	{
		FlushResourceBarriers();
		m_DynamicViewDescriptorHeap.CommitComputeRootDescriptorTables(m_CommandList);
		m_DynamicSamplerDescriptorHeap.CommitComputeRootDescriptorTables(m_CommandList);
		m_CommandList->Dispatch((UINT)GroupCountX, (UINT)GroupCountY, (UINT)GroupCountZ);
	}

	inline void ComputeContext::Dispatch1D(size_t ThreadCountX, size_t GroupSizeX)
	{
		Dispatch(D_MEMORY::DivideByMultiple(ThreadCountX, GroupSizeX), 1, 1);
	}

	inline void ComputeContext::Dispatch2D(size_t ThreadCountX, size_t ThreadCountY, size_t GroupSizeX, size_t GroupSizeY)
	{
		Dispatch(
			D_MEMORY::DivideByMultiple(ThreadCountX, GroupSizeX),
			D_MEMORY::DivideByMultiple(ThreadCountY, GroupSizeY), 1);
	}

	inline void ComputeContext::Dispatch3D(size_t ThreadCountX, size_t ThreadCountY, size_t ThreadCountZ, size_t GroupSizeX, size_t GroupSizeY, size_t GroupSizeZ)
	{
		Dispatch(
			D_MEMORY::DivideByMultiple(ThreadCountX, GroupSizeX),
			D_MEMORY::DivideByMultiple(ThreadCountY, GroupSizeY),
			D_MEMORY::DivideByMultiple(ThreadCountZ, GroupSizeZ));
	}

	inline void CommandContext::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr)
	{
		if (m_CurrentDescriptorHeaps[Type] != HeapPtr)
		{
			m_CurrentDescriptorHeaps[Type] = HeapPtr;
			BindDescriptorHeaps();
		}
	}

	inline void CommandContext::SetDescriptorHeaps(UINT HeapCount, D3D12_DESCRIPTOR_HEAP_TYPE Type[], ID3D12DescriptorHeap* HeapPtrs[])
	{
		bool AnyChanged = false;

		for (UINT i = 0; i < HeapCount; ++i)
		{
			if (m_CurrentDescriptorHeaps[Type[i]] != HeapPtrs[i])
			{
				m_CurrentDescriptorHeaps[Type[i]] = HeapPtrs[i];
				AnyChanged = true;
			}
		}

		if (AnyChanged)
			BindDescriptorHeaps();
	}

	inline void CommandContext::SetPredication(ID3D12Resource* Buffer, UINT64 BufferOffset, D3D12_PREDICATION_OP Op)
	{
		m_CommandList->SetPredication(Buffer, BufferOffset, Op);
	}

	inline void GraphicsContext::SetDynamicDescriptor(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle)
	{
		SetDynamicDescriptors(RootIndex, Offset, 1, &Handle);
	}

	inline void ComputeContext::SetDynamicDescriptor(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle)
	{
		SetDynamicDescriptors(RootIndex, Offset, 1, &Handle);
	}

	inline void GraphicsContext::SetDynamicDescriptors(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[])
	{
		m_DynamicViewDescriptorHeap.SetGraphicsDescriptorHandles(RootIndex, Offset, Count, Handles);
	}

	inline void ComputeContext::SetDynamicDescriptors(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[])
	{
		m_DynamicViewDescriptorHeap.SetComputeDescriptorHandles(RootIndex, Offset, Count, Handles);
	}

	inline void GraphicsContext::SetDynamicSampler(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle)
	{
		SetDynamicSamplers(RootIndex, Offset, 1, &Handle);
	}

	inline void GraphicsContext::SetDynamicSamplers(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[])
	{
		m_DynamicSamplerDescriptorHeap.SetGraphicsDescriptorHandles(RootIndex, Offset, Count, Handles);
	}

	inline void ComputeContext::SetDynamicSampler(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle)
	{
		SetDynamicSamplers(RootIndex, Offset, 1, &Handle);
	}

	inline void ComputeContext::SetDynamicSamplers(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[])
	{
		m_DynamicSamplerDescriptorHeap.SetComputeDescriptorHandles(RootIndex, Offset, Count, Handles);
	}

	inline void GraphicsContext::SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle)
	{
		m_CommandList->SetGraphicsRootDescriptorTable(RootIndex, FirstHandle);
	}

	inline void ComputeContext::SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle)
	{
		m_CommandList->SetComputeRootDescriptorTable(RootIndex, FirstHandle);
	}

	inline void GraphicsContext::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& IBView)
	{
		m_CommandList->IASetIndexBuffer(&IBView);
	}

	inline void GraphicsContext::SetVertexBuffer(UINT Slot, const D3D12_VERTEX_BUFFER_VIEW& VBView)
	{
		SetVertexBuffers(Slot, 1, &VBView);
	}

	inline void GraphicsContext::SetVertexBuffers(UINT StartSlot, UINT Count, const D3D12_VERTEX_BUFFER_VIEW VBViews[])
	{
		m_CommandList->IASetVertexBuffers(StartSlot, Count, VBViews);
	}

	inline void GraphicsContext::Draw(UINT VertexCount, UINT VertexStartOffset)
	{
		DrawInstanced(VertexCount, 1, VertexStartOffset, 0);
	}

	inline void GraphicsContext::DrawIndexed(UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
	{
		DrawIndexedInstanced(IndexCount, 1, StartIndexLocation, BaseVertexLocation, 0);
	}

	inline void GraphicsContext::DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount,
		UINT StartVertexLocation, UINT StartInstanceLocation)
	{
		FlushResourceBarriers();
		m_DynamicViewDescriptorHeap.CommitGraphicsRootDescriptorTables(m_CommandList);
		m_DynamicSamplerDescriptorHeap.CommitGraphicsRootDescriptorTables(m_CommandList);
		m_CommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
	}

	inline void GraphicsContext::DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation,
		INT BaseVertexLocation, UINT StartInstanceLocation)
	{
		FlushResourceBarriers();
		m_DynamicViewDescriptorHeap.CommitGraphicsRootDescriptorTables(m_CommandList);
		m_DynamicSamplerDescriptorHeap.CommitGraphicsRootDescriptorTables(m_CommandList);
		m_CommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
	}

	inline void GraphicsContext::ExecuteIndirect(CommandSignature& CommandSig,
		D_GRAPHICS_BUFFERS::GpuBuffer& ArgumentBuffer, uint64_t ArgumentStartOffset,
		uint32_t MaxCommands, D_GRAPHICS_BUFFERS::GpuBuffer* CommandCounterBuffer, uint64_t CounterOffset)
	{
		FlushResourceBarriers();
		m_DynamicViewDescriptorHeap.CommitGraphicsRootDescriptorTables(m_CommandList);
		m_DynamicSamplerDescriptorHeap.CommitGraphicsRootDescriptorTables(m_CommandList);
		m_CommandList->ExecuteIndirect(CommandSig.GetSignature(), MaxCommands,
			ArgumentBuffer.GetResource(), ArgumentStartOffset,
			CommandCounterBuffer == nullptr ? nullptr : CommandCounterBuffer->GetResource(), CounterOffset);
	}

	inline void GraphicsContext::DrawIndirect(D_GRAPHICS_BUFFERS::GpuBuffer& ArgumentBuffer, uint64_t ArgumentBufferOffset)
	{
		ExecuteIndirect(D_GRAPHICS::DrawIndirectCommandSignature, ArgumentBuffer, ArgumentBufferOffset);
	}

	inline void ComputeContext::ExecuteIndirect(CommandSignature& CommandSig,
		D_GRAPHICS_BUFFERS::GpuBuffer& ArgumentBuffer, uint64_t ArgumentStartOffset,
		uint32_t MaxCommands, D_GRAPHICS_BUFFERS::GpuBuffer* CommandCounterBuffer, uint64_t CounterOffset)
	{
		FlushResourceBarriers();
		m_DynamicViewDescriptorHeap.CommitComputeRootDescriptorTables(m_CommandList);
		m_DynamicSamplerDescriptorHeap.CommitComputeRootDescriptorTables(m_CommandList);
		m_CommandList->ExecuteIndirect(CommandSig.GetSignature(), MaxCommands,
			ArgumentBuffer.GetResource(), ArgumentStartOffset,
			CommandCounterBuffer == nullptr ? nullptr : CommandCounterBuffer->GetResource(), CounterOffset);
	}

	inline void ComputeContext::DispatchIndirect(D_GRAPHICS_BUFFERS::GpuBuffer& ArgumentBuffer, uint64_t ArgumentBufferOffset)
	{
		ExecuteIndirect(D_GRAPHICS::DispatchIndirectCommandSignature, ArgumentBuffer, ArgumentBufferOffset);
	}

	inline void CommandContext::CopyBuffer(D_GRAPHICS_UTILS::GpuResource& Dest, D_GRAPHICS_UTILS::GpuResource& Src)
	{
		TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST);
		TransitionResource(Src, D3D12_RESOURCE_STATE_COPY_SOURCE);
		FlushResourceBarriers();
		m_CommandList->CopyResource(Dest.GetResource(), Src.GetResource());
	}

	inline void CommandContext::CopyBufferRegion(D_GRAPHICS_UTILS::GpuResource& Dest, size_t DestOffset, D_GRAPHICS_UTILS::GpuResource& Src, size_t SrcOffset, size_t NumBytes)
	{
		TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST);
		//TransitionResource(Src, D3D12_RESOURCE_STATE_COPY_SOURCE);
		FlushResourceBarriers();
		m_CommandList->CopyBufferRegion(Dest.GetResource(), DestOffset, Src.GetResource(), SrcOffset, NumBytes);
	}

	inline void CommandContext::CopyCounter(D_GRAPHICS_UTILS::GpuResource& Dest, size_t DestOffset, D_GRAPHICS_BUFFERS::StructuredBuffer& Src)
	{
		TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST);
		TransitionResource(Src.GetCounterBuffer(), D3D12_RESOURCE_STATE_COPY_SOURCE);
		FlushResourceBarriers();
		m_CommandList->CopyBufferRegion(Dest.GetResource(), DestOffset, Src.GetCounterBuffer().GetResource(), 0, 4);
	}

	inline void CommandContext::CopyTextureRegion(D_GRAPHICS_UTILS::GpuResource& Dest, UINT x, UINT y, UINT z, D_GRAPHICS_UTILS::GpuResource& Source, RECT& Rect)
	{
		TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST);
		TransitionResource(Source, D3D12_RESOURCE_STATE_COPY_SOURCE);
		FlushResourceBarriers();

		D3D12_TEXTURE_COPY_LOCATION destLoc = CD3DX12_TEXTURE_COPY_LOCATION(Dest.GetResource(), 0);
		D3D12_TEXTURE_COPY_LOCATION srcLoc = CD3DX12_TEXTURE_COPY_LOCATION(Source.GetResource(), 0);

		D3D12_BOX box = {};
		box.back = 1;
		box.left = Rect.left;
		box.right = Rect.right;
		box.top = Rect.top;
		box.bottom = Rect.bottom;

		m_CommandList->CopyTextureRegion(&destLoc, x, y, z, &srcLoc, &box);
	}

	inline void CommandContext::ResetCounter(D_GRAPHICS_BUFFERS::StructuredBuffer& Buf, uint32_t Value)
	{
		FillBuffer(Buf.GetCounterBuffer(), 0, Value, sizeof(uint32_t));
		TransitionResource(Buf.GetCounterBuffer(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}

	inline void CommandContext::InsertTimeStamp(ID3D12QueryHeap* pQueryHeap, uint32_t QueryIdx)
	{
		m_CommandList->EndQuery(pQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, QueryIdx);
	}

	inline void CommandContext::ResolveTimeStamps(ID3D12Resource* pReadbackHeap, ID3D12QueryHeap* pQueryHeap, uint32_t NumQueries)
	{
		m_CommandList->ResolveQueryData(pQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, NumQueries, pReadbackHeap, 0);
	}
}