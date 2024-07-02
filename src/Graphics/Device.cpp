#include "pch.hpp"
#include "Device.hpp"

#include "Adapter.hpp"

#include <Job/Job.hpp>


namespace Darius::Graphics
{
	D3D12Device::D3D12Device(D3D12Adapter* adapter) :
		mAdapter(adapter),
		mResidencyManager(*this)
	{

		D_ASSERT(D_JOB::IsMainThread());

	}

	D3D12Device::~D3D12Device()
	{

	}

	D3D12Device::ResidencyManager::ResidencyManager(D3D12Device& parent)
	{
		IDXGIAdapter3* adapter3 = nullptr;
		parent.GetParentAdapter()->GetAdapter()->QueryInterface(IID_PPV_ARGS(&adapter3));
		
		uint32_t residencyMangerGPUIndex = 0; // We only support one gpu
		D3DX12Residency::InitializeResidencyManager(*this, parent.GetDevice(), residencyMangerGPUIndex, adapter3, RESIDENCY_PIPELINE_DEPTH);
	}

	D3D12Device::ResidencyManager::~ResidencyManager()
	{
		D3DX12Residency::DestroyResidencyManager(*this);
	}

	ID3D12Device* D3D12Device::GetDevice() const
	{
		return GetParentAdapter()->GetD3DDevice();
	}

	ID3D12Device5* D3D12Device::GetDevice5() const
	{
		return GetParentAdapter()->GetD3DDevice5();
	}

	ID3D12Device7* D3D12Device::GetDevice7() const
	{
		return GetParentAdapter()->GetD3DDevice7();
	}

	void D3D12Device::Setup()
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS FeatureData = {};
		if(D_HR_SUCCEEDED(GetDevice()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &FeatureData, sizeof(FeatureData))))
		{
			if(FeatureData.TypedUAVLoadAdditionalFormats)
			{
				D3D12_FEATURE_DATA_FORMAT_SUPPORT Support =
				{
					DXGI_FORMAT_R11G11B10_FLOAT, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE
				};

				if(D_HR_SUCCEEDED(GetDevice()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &Support, sizeof(Support))) &&
					(Support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
				{
					mTypedUAVLoadSupport_R11G11B10_FLOAT = true;
				}

				Support.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

				if(D_HR_SUCCEEDED(GetDevice()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &Support, sizeof(Support))) &&
					(Support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
				{
					mTypedUAVLoadSupport_R16G16B16A16_FLOAT = true;
				}
			}
		}

		D3D12_FEATURE_DATA_D3D12_OPTIONS5 FeatureData5 = {};
		if(D_HR_SUCCEEDED(GetDevice()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &FeatureData5, sizeof(FeatureData5))))
		{
			if(FeatureData5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
				mRaytracingSupport = true;
		}
	}

}