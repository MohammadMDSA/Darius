#pragma once

#include "GraphicsUtils/GpuResource.hpp"
#include "GraphicsUtils/Residency.hpp"

#include <Utils/Common.hpp>

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif // !D_GRAPHICS

// enough to ensure that the GPU is rarely blocked by residency work
#define RESIDENCY_PIPELINE_DEPTH	6

namespace Darius::Graphics
{

	class D3D12Adapter;

	class D3D12Device final : public NonCopyable
	{
	public:
		D3D12Device(D3D12Adapter* adapter);
		~D3D12Device();

		ID3D12Device* GetDevice() const;

		ID3D12Device5* GetDevice5() const;
		ID3D12Device7* GetDevice7() const;

		// Residency
		INLINE FD3D12ResidencyManager& GetResidencyManager() { return mResidencyManager; }

		D3D12Adapter* GetParentAdapter() const { return mAdapter.Get(); }
		void Setup();

	private:
		// called by SetupAfterDeviceCreation() when the device gets initialized

		struct ResidencyManager : public D3DX12Residency::ResidencyManager
		{
			ResidencyManager(D3D12Device& Parent);
			~ResidencyManager();
		} mResidencyManager;

		Microsoft::WRL::ComPtr<D3D12Adapter> mAdapter;

		uint32_t mTypedUAVLoadSupport_R11G11B10_FLOAT : 1 = false;
		uint32_t mTypedUAVLoadSupport_R16G16B16A16_FLOAT : 1 = false;
		uint32_t mRaytracingSupport : 1 = false;
	};
}