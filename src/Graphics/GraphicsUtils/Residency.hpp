#pragma once

#pragma warning(push)
#pragma warning(disable: 4555)
#pragma warning(disable: 5214)
#include "Graphics/d3dx12Residency.h"
#pragma warning(pop)

#include <Utils/Assert.hpp>

struct D3D12ResidencyHandle : public D3DX12Residency::ManagedObject
{ };

namespace D3DX12Residency
{
	inline void Initialize(D3D12ResidencyHandle& Object, ID3D12Pageable* pResource, uint64_t ObjectSize)
	{
		Object.Initialize(pResource, ObjectSize);
	}

	inline bool IsInitialized(D3D12ResidencyHandle& Object)
	{
		return Object.IsInitialized();
	}

	inline bool IsInitialized(D3D12ResidencyHandle* pObject)
	{
		return pObject && IsInitialized(*pObject);
	}

	inline void BeginTrackingObject(ResidencyManager& ResidencyManager, D3D12ResidencyHandle& Object)
	{
		ResidencyManager.BeginTrackingObject(&Object);
	}

	inline void EndTrackingObject(ResidencyManager& ResidencyManager, D3D12ResidencyHandle& Object)
	{
		ResidencyManager.EndTrackingObject(&Object);
	}

	inline void InitializeResidencyManager(ResidencyManager& ResidencyManager, ID3D12Device* Device, uint32_t GPUIndex, IDXGIAdapter3* Adapter, uint32_t MaxLatency)
	{
		D_ASSERT(D_HR_SUCCEEDED(ResidencyManager.Initialize(Device, GPUIndex, Adapter, MaxLatency)));
	}

	inline void DestroyResidencyManager(ResidencyManager& ResidencyManager)
	{
		ResidencyManager.Destroy();
	}

	inline ResidencySet* CreateResidencySet(ResidencyManager& ResidencyManager)
	{
		return ResidencyManager.CreateResidencySet();
	}

	inline void DestroyResidencySet(ResidencyManager& ResidencyManager, ResidencySet* pSet)
	{
		if(pSet)
		{
			ResidencyManager.DestroyResidencySet(pSet);
		}
	}

	inline void Open(ResidencySet* pSet)
	{
		if(pSet)
		{
			D_ASSERT(D_HR_SUCCEEDED(pSet->Open()));
		}
	}

	inline void Close(ResidencySet* pSet)
	{
		if(pSet)
		{
			D_ASSERT(D_HR_SUCCEEDED(pSet->Close()));
		}
	}

	inline void Insert(ResidencySet& Set, D3D12ResidencyHandle& Object)
	{
		D_ASSERT(Object.IsInitialized());
		Set.Insert(&Object);
	}

	inline void Insert(ResidencySet& Set, D3D12ResidencyHandle* pObject)
	{
		D_ASSERT(pObject && pObject->IsInitialized());
		Set.Insert(pObject);
	}
}

typedef D3DX12Residency::ResidencySet D3D12ResidencySet;
typedef D3DX12Residency::ResidencyManager D3D12ResidencyManager;