#include "pch.hpp"
#include "Adapter.hpp"

#include <Job/Job.hpp>
#include <Math/VectorMath.hpp>

namespace
{
	bool DebugLayerEnabled = false;
	bool GpuBasedValidation = false;
}
namespace Darius::Graphics
{
	static D3D_FEATURE_LEVEL FindHighestFeatureLevel(ID3D12Device* Device, D3D_FEATURE_LEVEL MinFeatureLevel)
	{
		const D3D_FEATURE_LEVEL FeatureLevels[] =
		{
			// Add new feature levels that the app supports here.
			D3D_FEATURE_LEVEL_12_2,
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0
		};

		// Determine the max feature level supported by the driver and hardware.
		D3D12_FEATURE_DATA_FEATURE_LEVELS FeatureLevelCaps {};
		FeatureLevelCaps.pFeatureLevelsRequested = FeatureLevels;
		FeatureLevelCaps.NumFeatureLevels = _countof(FeatureLevels);

		if(SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &FeatureLevelCaps, sizeof(FeatureLevelCaps))))
		{
			return FeatureLevelCaps.MaxSupportedFeatureLevel;
		}

		return MinFeatureLevel;
	}

	D3D12AdapterDesc::D3D12AdapterDesc() = default;

	D3D12AdapterDesc::D3D12AdapterDesc(const DXGI_ADAPTER_DESC& InDesc, int32_t InAdapterIndex, D3D12DeviceBasicInfo const& DeviceInfo)
		: Desc(InDesc)
		, AdapterIndex(InAdapterIndex)
		, MaxSupportedFeatureLevel(DeviceInfo.MaxFeatureLevel)
		, MaxSupportedShaderModel(DeviceInfo.MaxShaderModel)
		, ResourceBindingTier(DeviceInfo.ResourceBindingTier)
		, ResourceHeapTier(DeviceInfo.ResourceHeapTier)
		, SupportsWaveOps(DeviceInfo.SupportsWaveOps)
		, SupportsAtomic64(DeviceInfo.SupportsAtomic64)
	{

	}

	bool D3D12AdapterDesc::IsValid() const
	{
		return MaxSupportedFeatureLevel != (D3D_FEATURE_LEVEL)0 && AdapterIndex >= 0;
	}

	HRESULT D3D12AdapterDesc::EnumAdapters(int32_t AdapterIndex, DXGI_GPU_PREFERENCE GpuPreference, IDXGIFactory2* DxgiFactory2, IDXGIFactory6* DxgiFactory6, IDXGIAdapter** TempAdapter)
	{
		if(!DxgiFactory6 || GpuPreference == DXGI_GPU_PREFERENCE_UNSPECIFIED)
		{
			return DxgiFactory2->EnumAdapters(AdapterIndex, TempAdapter);
		}
		else
		{
			return DxgiFactory6->EnumAdapterByGpuPreference(AdapterIndex, GpuPreference, IID_PPV_ARGS(TempAdapter));
		}
	}

	HRESULT D3D12AdapterDesc::EnumAdapters(IDXGIFactory2* DxgiFactory2, IDXGIFactory6* DxgiFactory6, IDXGIAdapter** TempAdapter) const
	{
		return EnumAdapters(AdapterIndex, GpuPreference, DxgiFactory2, DxgiFactory6, TempAdapter);
	}

	D3D12Adapter::D3D12Adapter(D3D12AdapterDesc& DescIn)
		: mDesc(DescIn)
	{
		uint32_t MaxGPUCount = 1; // By default, multi-gpu is disabled.

		mDesc.NumDeviceNodes = D_MATH::Min(D_MATH::Min(mDesc.NumDeviceNodes, MaxGPUCount), (uint32_t)MAX_NUM_GPUS);
	}


	D3D12Adapter::~D3D12Adapter()
	{
		// trace all leak D3D resource
		if(mDXGIDebug != nullptr)
		{
			mDXGIDebug->ReportLiveObjects(
				GUID {0xe48ae283, 0xda80, 0x490b, { 0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8 }}, // DXGI_DEBUG_ALL
				DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
			mDXGIDebug.Reset();

		}

		if(GetD3DDevice() && DebugLayerEnabled)
		{
			Microsoft::WRL::ComPtr<ID3D12DebugDevice> Debug;
			if(D_HR_SUCCEEDED(GetD3DDevice()->QueryInterface(IID_PPV_ARGS(Debug.GetAddressOf()))))
			{
				D3D12_RLDO_FLAGS rldoFlags = D3D12_RLDO_DETAIL;
				Debug->ReportLiveDeviceObjects(rldoFlags);
			}
		}

		mDevice.Reset();
		mRootDevice.Reset();
	}

	void D3D12Adapter::CreateRootDevice(bool withDebug)
	{

		bool initFactoryWithDebug = false;

#if _DEBUG
		if(D_HR_SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(mDebugController.ReleaseAndGetAddressOf()))))
		{
			if(withDebug)
				mDebugController->EnableDebugLayer();
		}
		else
		{
			D_LOG_WARN("Direct3D Debug Device is not available");
		}



#define AddDebugDevice(version) \
	if(D_HR_SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(mDebugController##version.ReleaseAndGetAddressOf())))) \
	{ \
		D_LOG_INFO("Direct3D Debug Device " #version " is present."); \
	}

		AddDebugDevice(1);
		AddDebugDevice(2);
		AddDebugDevice(3);
		AddDebugDevice(4);
		AddDebugDevice(5);
		AddDebugDevice(6);

		if(mDebugController1 && GpuBasedValidation)
		{
			mDebugController1->SetEnableGPUBasedValidation(true);
		}

#undef AddDebugDevice

		Microsoft::WRL::ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if(SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
		{
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

			initFactoryWithDebug = true;

			DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
			{
				80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
			};
			DXGI_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
			filter.DenyList.pIDList = hide;
			dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
		}
#endif

		CreateDXGIFactory(initFactoryWithDebug);

		EnumAdapters(mDxgiAdapter.ReleaseAndGetAddressOf());

		D_ASSERT(D_HR_SUCCEEDED(D3D12CreateDevice(GetAdapter(), GetFeatureLevel(), IID_PPV_ARGS(mRootDevice.ReleaseAndGetAddressOf()))));

#if _DEBUG
		Microsoft::WRL::ComPtr<ID3D12InfoQueue> d3dInfoQueue;
		if(D_HR_SUCCEEDED(mRootDevice.As(&d3dInfoQueue)))
		{
			d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			D3D12_MESSAGE_ID hide[] =
			{
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
				// Workarounds for debug layer issues on hybrid-graphics systems
				D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
				D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
				D3D12_MESSAGE_ID_RESOLVE_QUERY_INVALID_QUERY_STATE
			};
			D3D12_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);
		}
#endif

	}

	void D3D12Adapter::InitializeDevices()
	{
		D_ASSERT(D_JOB::IsMainThread());

		// Use a debug device if specified on the command line.
		bool bWithD3DDebug = DebugLayerEnabled;

		// If we don't have a device yet, either because this is the first viewport, or the old device was removed, create a device.
		if(!mRootDevice)
		{
			CreateRootDevice(bWithD3DDebug);

			// See if we can get any newer device interfaces (to use newer D3D12 features).

			if(SUCCEEDED(mRootDevice->QueryInterface(IID_PPV_ARGS(mRootDevice1.GetAddressOf()))))
			{
				D_LOG_WARN("ID3D12Device1 is supported.");
			}
			if(SUCCEEDED(mRootDevice->QueryInterface(IID_PPV_ARGS(mRootDevice2.GetAddressOf()))))
			{
				D_LOG_WARN("ID3D12Device2 is supported.");
			}

			if(mRootDevice1 == nullptr || mRootDevice2 == nullptr)
			{
				// Note: we require Windows 1703 in FD3D12DynamicRHIModule::IsSupported()
				// If we still lack support, the user's drivers could be out of date.
				D_LOG_WARN("Missing full support for Direct3D 12. Please update to the latest drivers.");
			}
			if(SUCCEEDED(mRootDevice->QueryInterface(IID_PPV_ARGS(mRootDevice3.GetAddressOf()))))
			{
				D_LOG_WARN("ID3D12Device3 is supported.");
			}
			if(SUCCEEDED(mRootDevice->QueryInterface(IID_PPV_ARGS(mRootDevice4.GetAddressOf()))))
			{
				D_LOG_WARN("ID3D12Device4 is supported.");
			}
			if(SUCCEEDED(mRootDevice->QueryInterface(IID_PPV_ARGS(mRootDevice5.GetAddressOf()))))
			{
				D_LOG_WARN("ID3D12Device5 is supported.");
			}
			if(SUCCEEDED(mRootDevice->QueryInterface(IID_PPV_ARGS(mRootDevice6.GetAddressOf()))))
			{
				D_LOG_WARN("ID3D12Device6 is supported.");
			}
			if(SUCCEEDED(mRootDevice->QueryInterface(IID_PPV_ARGS(mRootDevice7.GetAddressOf()))))
			{
				D_LOG_WARN("ID3D12Device7 is supported.");
			}
			if(SUCCEEDED(mRootDevice->QueryInterface(IID_PPV_ARGS(mRootDevice8.GetAddressOf()))))
			{
				D_LOG_WARN("ID3D12Device8 is supported.");

				// D3D12_HEAP_FLAG_CREATE_NOT_ZEROED is supported
				mHeapNotZeroedSupported = true;
			}
			if(SUCCEEDED(mRootDevice->QueryInterface(IID_PPV_ARGS(mRootDevice9.GetAddressOf()))))
			{
				D_LOG_WARN("ID3D12Device9 is supported.");
			}
			if(SUCCEEDED(mRootDevice->QueryInterface(IID_PPV_ARGS(mRootDevice10.GetAddressOf()))))
			{
				D_LOG_WARN("ID3D12Device10 is supported.");
			}

			D3D12_FEATURE_DATA_D3D12_OPTIONS19 Features19 {};
			if(SUCCEEDED(mRootDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS19, &Features19, sizeof(Features19))))
			{
				mMaxNonSamplerDescriptors = Features19.MaxViewDescriptorHeapSize;
				mMaxSamplerDescriptors = Features19.MaxSamplerDescriptorHeapSizeWithStaticSamplers;
			}
			else if(GetResourceBindingTier() == D3D12_RESOURCE_BINDING_TIER_1)
			{
				mMaxNonSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1;
				mMaxSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
			}
			else if(GetResourceBindingTier() == D3D12_RESOURCE_BINDING_TIER_2)
			{
				mMaxNonSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2;
				mMaxSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
			}
			else
			{
				mMaxNonSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2;
				mMaxSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
			}

			mDevice = new D3D12Device(this);
			mDevice->Setup();
		}
	}

	void D3D12Adapter::CreateDXGIFactory(bool bWithDebug)
	{
		HMODULE UsedDxgiDllHandle = (HMODULE)0;

		DWORD dxgiFactoryFlag = bWithDebug ? DXGI_CREATE_FACTORY_DEBUG : 0;
		D_ASSERT(D_HR_SUCCEEDED(CreateDXGIFactory2(dxgiFactoryFlag, IID_PPV_ARGS(mDxgiFactory2.ReleaseAndGetAddressOf()))));

		InitDXGIFactoryVariants(mDxgiFactory2.Get());
	}

	void D3D12Adapter::InitDXGIFactoryVariants(IDXGIFactory2* InDxgiFactory2)
	{
		InDxgiFactory2->QueryInterface(IID_PPV_ARGS(mDxgiFactory3.GetAddressOf()));
		InDxgiFactory2->QueryInterface(IID_PPV_ARGS(mDxgiFactory4.GetAddressOf()));
		InDxgiFactory2->QueryInterface(IID_PPV_ARGS(mDxgiFactory5.GetAddressOf()));
		InDxgiFactory2->QueryInterface(IID_PPV_ARGS(mDxgiFactory6.GetAddressOf()));
		InDxgiFactory2->QueryInterface(IID_PPV_ARGS(mDxgiFactory7.GetAddressOf()));
	}

	HRESULT D3D12Adapter::EnumAdapters(IDXGIAdapter** TempAdapter) const
	{
		return D3D12AdapterDesc::EnumAdapters(mDesc.AdapterIndex, mDesc.GpuPreference, mDxgiFactory2.Get(), mDxgiFactory6.Get(), TempAdapter);
	}
}