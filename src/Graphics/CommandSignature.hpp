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

#include <Utils/Assert.hpp>

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

namespace Darius::Graphics::Utils
{
	class RootSignature;
}

namespace Darius::Graphics
{

	class IndirectParameter
	{
		friend class CommandSignature;
	public:

		IndirectParameter()
		{
			m_IndirectParam.Type = (D3D12_INDIRECT_ARGUMENT_TYPE)0xFFFFFFFF;
		}

		void Draw(void)
		{
			m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
		}

		void DrawIndexed(void)
		{
			m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
		}

		void Dispatch(void)
		{
			m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
		}

		void VertexBufferView(UINT Slot)
		{
			m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
			m_IndirectParam.VertexBuffer.Slot = Slot;
		}

		void IndexBufferView(void)
		{
			m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
		}

		void Constant(UINT RootParameterIndex, UINT DestOffsetIn32BitValues, UINT Num32BitValuesToSet)
		{
			m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
			m_IndirectParam.Constant.RootParameterIndex = RootParameterIndex;
			m_IndirectParam.Constant.DestOffsetIn32BitValues = DestOffsetIn32BitValues;
			m_IndirectParam.Constant.Num32BitValuesToSet = Num32BitValuesToSet;
		}

		void ConstantBufferView(UINT RootParameterIndex)
		{
			m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
			m_IndirectParam.ConstantBufferView.RootParameterIndex = RootParameterIndex;
		}

		void ShaderResourceView(UINT RootParameterIndex)
		{
			m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
			m_IndirectParam.ShaderResourceView.RootParameterIndex = RootParameterIndex;
		}

		void UnorderedAccessView(UINT RootParameterIndex)
		{
			m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW;
			m_IndirectParam.UnorderedAccessView.RootParameterIndex = RootParameterIndex;
		}

		const D3D12_INDIRECT_ARGUMENT_DESC& GetDesc(void) const { return m_IndirectParam; }

	protected:

		D3D12_INDIRECT_ARGUMENT_DESC m_IndirectParam;
	};

	class CommandSignature
	{
	public:

		CommandSignature(UINT NumParams = 0) : mFinalized(FALSE), mNumParameters(NumParams)
		{
			Reset(NumParams);
		}

		void Destroy(void)
		{
			mSignature = nullptr;
			mParamArray = nullptr;
		}

		void Reset(UINT NumParams)
		{
			if (NumParams > 0)
				mParamArray.reset(new IndirectParameter[NumParams]);
			else
				mParamArray = nullptr;

			mNumParameters = NumParams;
		}

		IndirectParameter& operator[] (size_t EntryIndex)
		{
			D_ASSERT(EntryIndex < mNumParameters);
			return mParamArray.get()[EntryIndex];
		}

		const IndirectParameter& operator[] (size_t EntryIndex) const
		{
			D_ASSERT(EntryIndex < mNumParameters);
			return mParamArray.get()[EntryIndex];
		}

		void Finalize(const Darius::Graphics::Utils::RootSignature* RootSignature = nullptr);

		ID3D12CommandSignature* GetSignature() const { return mSignature.Get(); }

	protected:

		BOOL mFinalized;
		UINT mNumParameters;
		std::unique_ptr<IndirectParameter[]> mParamArray;
		Microsoft::WRL::ComPtr<ID3D12CommandSignature> mSignature;
	};
}