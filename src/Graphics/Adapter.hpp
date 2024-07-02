#pragma once

#include "Device.hpp"

#include <Core/Containers/Array.hpp>
#include <Utils/Common.hpp>

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif // !D_GRAPHICS

#define MAX_NUM_GPUS 1

namespace Darius::Graphics
{

	struct D3D12DeviceBasicInfo
	{
		D3D_FEATURE_LEVEL           MaxFeatureLevel;
		D3D_SHADER_MODEL            MaxShaderModel;
		D3D12_RESOURCE_BINDING_TIER ResourceBindingTier;
		D3D12_RESOURCE_HEAP_TIER    ResourceHeapTier;
		uint32_t                    NumDeviceNodes;
		bool                        SupportsWaveOps;
		bool                        SupportsAtomic64;
	};

	struct D3D12AdapterDesc
	{
		D3D12AdapterDesc();
		D3D12AdapterDesc(const DXGI_ADAPTER_DESC& InDesc, int32_t InAdapterIndex, D3D12DeviceBasicInfo const& DeviceInfo);

		bool IsValid() const;

		static HRESULT EnumAdapters(int32_t AdapterIndex, DXGI_GPU_PREFERENCE GpuPreference, IDXGIFactory2* DxgiFactory2, IDXGIFactory6* DxgiFactory6, IDXGIAdapter** TempAdapter);
		HRESULT EnumAdapters(IDXGIFactory2* DxgiFactory2, IDXGIFactory6* DxgiFactory6, IDXGIAdapter** TempAdapter) const;

		DXGI_ADAPTER_DESC Desc {};

		/** -1 if not supported or FindAdapter() wasn't called. Ideally we would store a pointer to IDXGIAdapter but it's unlikely the adpaters change during engine init. */
		int32_t AdapterIndex = -1;

		/** The maximum D3D12 feature level supported. 0 if not supported or FindAdapter() wasn't called */
		D3D_FEATURE_LEVEL MaxSupportedFeatureLevel = (D3D_FEATURE_LEVEL)0;

		/** The maximum Shader Model supported. 0 if not supported or FindAdpater() wasn't called */
		D3D_SHADER_MODEL MaxSupportedShaderModel = (D3D_SHADER_MODEL)0;

		D3D12_RESOURCE_BINDING_TIER ResourceBindingTier = D3D12_RESOURCE_BINDING_TIER_1;

		D3D12_RESOURCE_HEAP_TIER ResourceHeapTier = D3D12_RESOURCE_HEAP_TIER_1;

		/** Number of device nodes (read: GPUs) */
		uint32_t NumDeviceNodes = 1;

		/** Whether the GPU is integrated or discrete. */
		bool IsIntegrated = false;

		/** Whether SM6.0 wave ops are supported */
		bool SupportsWaveOps = false;

		/** Whether SM6.6 atomic64 wave ops are supported */
		bool SupportsAtomic64 = false;

		DXGI_GPU_PREFERENCE GpuPreference = DXGI_GPU_PREFERENCE_UNSPECIFIED;
	};


	class D3D12Adapter : public NonCopyable
	{
	public:

		D3D12Adapter(D3D12AdapterDesc& DescIn);
		virtual ~D3D12Adapter();

		void InitializeDevices();

		virtual void CreateRootDevice(bool bWithDebug);

		void CreateDXGIFactory(bool bWithDebug);
		void InitDXGIFactoryVariants(IDXGIFactory2* InDxgiFactory2);
		HRESULT EnumAdapters(IDXGIAdapter** TempAdapter) const;

		INLINE D3D12Device* GetDevice() const { return mDevice.Get(); }

		INLINE ID3D12Device* GetD3DDevice() const { return mRootDevice.Get(); }
		INLINE ID3D12Device1* GetD3DDevice1() const { return mRootDevice1.Get(); }
		INLINE ID3D12Device2* GetD3DDevice2() const { return mRootDevice2.Get(); }
		INLINE ID3D12Device3* GetD3DDevice3() const { return mRootDevice3.Get(); }
		INLINE ID3D12Device4* GetD3DDevice4() const { return mRootDevice4.Get(); }
		INLINE ID3D12Device5* GetD3DDevice5() const { return mRootDevice5.Get(); }
		INLINE ID3D12Device6* GetD3DDevice6() const { return mRootDevice6.Get(); }
		INLINE ID3D12Device7* GetD3DDevice7() const { return mRootDevice7.Get(); }
		INLINE ID3D12Device8* GetD3DDevice8() const { return mRootDevice8.Get(); }
		INLINE ID3D12Device9* GetD3DDevice9() const { return mRootDevice9.Get(); }
		INLINE ID3D12Device10* GetD3DDevice10() const { return mRootDevice10.Get(); }
		
		INLINE IDXGIFactory2* GetDXGIFactory2() const { return mDxgiFactory2.Get(); }
		INLINE IDXGIFactory3* GetDXGIFactory3() const { return mDxgiFactory3.Get(); }
		INLINE IDXGIFactory4* GetDXGIFactory4() const { return mDxgiFactory4.Get(); }
		INLINE IDXGIFactory5* GetDXGIFactory5() const { return mDxgiFactory5.Get(); }
		INLINE IDXGIFactory6* GetDXGIFactory6() const { return mDxgiFactory6.Get(); }
		INLINE IDXGIFactory7* GetDXGIFactory7() const { return mDxgiFactory7.Get(); }

		INLINE ID3D12Debug* GetDebugController() const { return mDebugController.Get(); }
		INLINE ID3D12Debug1* GetDebugController1() const { return mDebugController1.Get();
		}
		INLINE ID3D12Debug2* GetDebugController2() const { return mDebugController2.Get(); }
		INLINE ID3D12Debug3* GetDebugController3() const { return mDebugController3.Get(); }
		INLINE ID3D12Debug4* GetDebugController4() const { return mDebugController4.Get(); }
		INLINE ID3D12Debug5* GetDebugController5() const { return mDebugController5.Get(); }
		INLINE ID3D12Debug6* GetDebugController6() const { return mDebugController6.Get(); }

		INLINE uint32_t GetAdapterIndex() const { return mDesc.AdapterIndex; }
		INLINE D3D_FEATURE_LEVEL GetFeatureLevel() const { return mDesc.MaxSupportedFeatureLevel; }
		INLINE D3D_SHADER_MODEL GetHighestShaderModel() const { return mDesc.MaxSupportedShaderModel; }

		INLINE D3D12_RESOURCE_HEAP_TIER     GetResourceHeapTier() const { return mDesc.ResourceHeapTier; }
		INLINE D3D12_RESOURCE_BINDING_TIER  GetResourceBindingTier() const { return mDesc.ResourceBindingTier; }

		INLINE bool IsDepthBoundsTestSupported() const { return mDepthBoundsTestSupported; }
		INLINE bool IsHeapNotZeroedSupported() const { return mHeapNotZeroedSupported; }

		INLINE IDXGIAdapter* GetAdapter() const { return mDxgiAdapter.Get(); }

	protected:
		Microsoft::WRL::ComPtr<ID3D12Device> mRootDevice;
		Microsoft::WRL::ComPtr<ID3D12Device1> mRootDevice1;
		Microsoft::WRL::ComPtr<ID3D12Device2> mRootDevice2;
		Microsoft::WRL::ComPtr<ID3D12Device3> mRootDevice3;
		Microsoft::WRL::ComPtr<ID3D12Device4> mRootDevice4;
		Microsoft::WRL::ComPtr<ID3D12Device5> mRootDevice5;
		Microsoft::WRL::ComPtr<ID3D12Device6> mRootDevice6;
		Microsoft::WRL::ComPtr<ID3D12Device7> mRootDevice7;
		Microsoft::WRL::ComPtr<ID3D12Device8> mRootDevice8;
		Microsoft::WRL::ComPtr<ID3D12Device9> mRootDevice9;
		Microsoft::WRL::ComPtr<ID3D12Device10> mRootDevice10;

		Microsoft::WRL::ComPtr<IDXGIFactory2> mDxgiFactory2;
		Microsoft::WRL::ComPtr<IDXGIFactory3> mDxgiFactory3;
		Microsoft::WRL::ComPtr<IDXGIFactory4> mDxgiFactory4;
		Microsoft::WRL::ComPtr<IDXGIFactory5> mDxgiFactory5;
		Microsoft::WRL::ComPtr<IDXGIFactory6> mDxgiFactory6;
		Microsoft::WRL::ComPtr<IDXGIFactory7> mDxgiFactory7;

#if _DEBUG
		Microsoft::WRL::ComPtr<IDXGIDebug> mDXGIDebug;
		Microsoft::WRL::ComPtr<ID3D12Debug> mDebugController;
		Microsoft::WRL::ComPtr<ID3D12Debug1> mDebugController1 ;
		Microsoft::WRL::ComPtr<ID3D12Debug2> mDebugController2;
		Microsoft::WRL::ComPtr<ID3D12Debug3> mDebugController3;
		Microsoft::WRL::ComPtr<ID3D12Debug4> mDebugController4;
		Microsoft::WRL::ComPtr<ID3D12Debug5> mDebugController5;
		Microsoft::WRL::ComPtr<ID3D12Debug6> mDebugController6;
#endif


		bool mDepthBoundsTestSupported = false;
		bool mCopyQueueTimestampQueriesSupported = false;
		bool mHeapNotZeroedSupported = false;

		int32_t mMaxNonSamplerDescriptors = 0;
		int32_t mMaxSamplerDescriptors = 0;

	private:
		D3D12AdapterDesc mDesc;

		Microsoft::WRL::ComPtr<D3D12Device> mDevice;

		Microsoft::WRL::ComPtr<IDXGIAdapter> mDxgiAdapter;
	};


	static bool IsRenderDocPresent(ID3D12Device* Device)
	{
		IID RenderDocID;
		if(SUCCEEDED(IIDFromString(L"{A7AA6116-9C8D-4BBA-9083-B4D816B71B78}", &RenderDocID)))
		{
			Microsoft::WRL::ComPtr<IUnknown> RenderDoc;
			if(SUCCEEDED(Device->QueryInterface(RenderDocID, (void**)RenderDoc.GetAddressOf())))
			{
				return true;
			}
		}

		return false;
	}

}