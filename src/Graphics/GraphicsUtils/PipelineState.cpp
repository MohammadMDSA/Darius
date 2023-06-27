#include "Graphics/pch.hpp"
#include "PipelineState.hpp"
#include "RootSignature.hpp"
#include "Graphics/GraphicsCore.hpp"
#include "Graphics/GraphicsDeviceManager.hpp"

#include <Core/Hash.hpp>
#include <Core/Memory/Memory.hpp>

#include <map>
#include <thread>
#include <mutex>

using namespace D_MEMORY;
using Microsoft::WRL::ComPtr;
using namespace std;

static map< size_t, ComPtr<ID3D12PipelineState> > s_GraphicsPSOHashMap;
static map< size_t, ComPtr<ID3D12PipelineState> > s_ComputePSOHashMap;

namespace Darius::Graphics::Utils
{
	void PSO::DestroyAll(void)
	{
		s_GraphicsPSOHashMap.clear();
		s_ComputePSOHashMap.clear();
	}


	GraphicsPSO::GraphicsPSO(const wchar_t* Name)
		: PSO(Name)
	{
		ZeroMemory(&mPSODesc, sizeof(mPSODesc));
		mPSODesc.NodeMask = 1;
		mPSODesc.SampleMask = 0xFFFFFFFFu;
		mPSODesc.SampleDesc.Count = 1;
		mPSODesc.InputLayout.NumElements = 0;
	}

	void GraphicsPSO::SetBlendState(const D3D12_BLEND_DESC& BlendDesc)
	{
		mPSODesc.BlendState = BlendDesc;
	}

	void GraphicsPSO::SetRasterizerState(const D3D12_RASTERIZER_DESC& RasterizerDesc)
	{
		mPSODesc.RasterizerState = RasterizerDesc;
	}

	void GraphicsPSO::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc)
	{
		mPSODesc.DepthStencilState = DepthStencilDesc;
	}

	void GraphicsPSO::SetSampleMask(UINT SampleMask)
	{
		mPSODesc.SampleMask = SampleMask;
	}

	void GraphicsPSO::SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType)
	{
		D_ASSERT_M(TopologyType != D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED, "Can't draw with undefined topology");
		mPSODesc.PrimitiveTopologyType = TopologyType;
	}

	void GraphicsPSO::SetPrimitiveRestart(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBProps)
	{
		mPSODesc.IBStripCutValue = IBProps;
	}

	void GraphicsPSO::SetDepthTargetFormat(DXGI_FORMAT DSVFormat, UINT MsaaCount, UINT MsaaQuality)
	{
		SetRenderTargetFormats(0, nullptr, DSVFormat, MsaaCount, MsaaQuality);
	}

	void GraphicsPSO::SetRenderTargetFormat(DXGI_FORMAT RTVFormat, DXGI_FORMAT DSVFormat, UINT MsaaCount, UINT MsaaQuality)
	{
		SetRenderTargetFormats(1, &RTVFormat, DSVFormat, MsaaCount, MsaaQuality);
	}

	void GraphicsPSO::SetRenderTargetFormats(UINT NumRTVs, const DXGI_FORMAT* RTVFormats, DXGI_FORMAT DSVFormat, UINT MsaaCount, UINT MsaaQuality)
	{
		D_ASSERT_M(NumRTVs == 0 || RTVFormats != nullptr, "Null format array conflicts with non-zero length");
		for (UINT i = 0; i < NumRTVs; ++i)
		{
			D_ASSERT(RTVFormats[i] != DXGI_FORMAT_UNKNOWN);
			mPSODesc.RTVFormats[i] = RTVFormats[i];
		}
		for (UINT i = NumRTVs; i < mPSODesc.NumRenderTargets; ++i)
			mPSODesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
		mPSODesc.NumRenderTargets = NumRTVs;
		mPSODesc.DSVFormat = DSVFormat;
		mPSODesc.SampleDesc.Count = MsaaCount;
		mPSODesc.SampleDesc.Quality = MsaaQuality;
	}

	void GraphicsPSO::SetInputLayout(UINT NumElements, const D3D12_INPUT_ELEMENT_DESC* pInputElementDescs)
	{
		mPSODesc.InputLayout.NumElements = NumElements;

		if (NumElements > 0)
		{
			D3D12_INPUT_ELEMENT_DESC* NewElements = (D3D12_INPUT_ELEMENT_DESC*)malloc(sizeof(D3D12_INPUT_ELEMENT_DESC) * NumElements);
			memcpy(NewElements, pInputElementDescs, NumElements * sizeof(D3D12_INPUT_ELEMENT_DESC));
			mInputLayouts.reset((const D3D12_INPUT_ELEMENT_DESC*)NewElements);
		}
		else
			mInputLayouts = nullptr;
	}

	void GraphicsPSO::Finalize(std::wstring nameOverride)
	{
		if (nameOverride != L"")
			mName = nameOverride.c_str();

		// Make sure the root signature is finalized first
		mPSODesc.pRootSignature = mRootSignature->GetSignature();
		D_ASSERT(mPSODesc.pRootSignature != nullptr);

		mPSODesc.InputLayout.pInputElementDescs = nullptr;
		size_t HashCode = D_CORE::HashState(&mPSODesc);
		HashCode = D_CORE::HashState(mInputLayouts.get(), mPSODesc.InputLayout.NumElements, HashCode);
		mPSODesc.InputLayout.pInputElementDescs = mInputLayouts.get();

		ID3D12PipelineState** PSORef = nullptr;
		bool firstCompile = false;
		{
			static mutex s_HashMapMutex;
			lock_guard<mutex> CS(s_HashMapMutex);
			auto iter = s_GraphicsPSOHashMap.find(HashCode);

			// Reserve space so the next inquiry will find that someone got here first.
			if (iter == s_GraphicsPSOHashMap.end())
			{
				firstCompile = true;
				PSORef = s_GraphicsPSOHashMap[HashCode].GetAddressOf();
			}
			else
				PSORef = iter->second.GetAddressOf();
		}

		if (firstCompile)
		{
			D_ASSERT(mPSODesc.DepthStencilState.DepthEnable != (mPSODesc.DSVFormat == DXGI_FORMAT_UNKNOWN));
			D_HR_CHECK(D_GRAPHICS_DEVICE::GetDevice()->CreateGraphicsPipelineState(&mPSODesc, IID_PPV_ARGS(&mPSO)));
			s_GraphicsPSOHashMap[HashCode].Attach(mPSO);
			mPSO->SetName(mName);
		}
		else
		{
			while (*PSORef == nullptr)
				this_thread::yield();
			mPSO = *PSORef;
		}
	}

	void ComputePSO::Finalize()
	{
		// Make sure the root signature is finalized first
		mPSODesc.pRootSignature = mRootSignature->GetSignature();
		D_ASSERT(mPSODesc.pRootSignature != nullptr);

		size_t HashCode = D_CORE::HashState(&mPSODesc);

		ID3D12PipelineState** PSORef = nullptr;
		bool firstCompile = false;
		{
			static mutex s_HashMapMutex;
			lock_guard<mutex> CS(s_HashMapMutex);
			auto iter = s_ComputePSOHashMap.find(HashCode);

			// Reserve space so the next inquiry will find that someone got here first.
			if (iter == s_ComputePSOHashMap.end())
			{
				firstCompile = true;
				PSORef = s_ComputePSOHashMap[HashCode].GetAddressOf();
			}
			else
				PSORef = iter->second.GetAddressOf();
		}

		if (firstCompile)
		{
			D_HR_CHECK(D_GRAPHICS_DEVICE::GetDevice()->CreateComputePipelineState(&mPSODesc, IID_PPV_ARGS(&mPSO)));
			s_ComputePSOHashMap[HashCode].Attach(mPSO);
			mPSO->SetName(mName);
		}
		else
		{
			while (*PSORef == nullptr)
				this_thread::yield();
			mPSO = *PSORef;
		}
	}

	ComputePSO::ComputePSO(const wchar_t* Name)
		: PSO(Name)
	{
		ZeroMemory(&mPSODesc, sizeof(mPSODesc));
		mPSODesc.NodeMask = 1;
	}
}