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
// Author(s):  James Stanard
//             Alex Nankervis
//

#include "Renderer/pch.hpp"
#include "SamplerManager.hpp"
#include "Renderer/GraphicsCore.hpp"
#include "Renderer/RenderDeviceManager.hpp"

#include <Core/Hash.hpp>
#include <Utils/Assert.hpp>

#include <map>

using namespace std;

namespace
{
	map<size_t, D3D12_CPU_DESCRIPTOR_HANDLE> s_SamplerCache;
}

namespace Darius::Graphics::Utils
{
	D3D12_CPU_DESCRIPTOR_HANDLE SamplerDesc::CreateDescriptor()
	{
		size_t hashValue = D_CORE::HashState(this);
		auto iter = s_SamplerCache.find(hashValue);
		if (iter != s_SamplerCache.end())
		{
			return iter->second;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE Handle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		D_RENDERER_DEVICE::GetDevice()->CreateSampler(this, Handle);
		return Handle;
	}

	void SamplerDesc::CreateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE Handle)
	{
		D_ASSERT(Handle.ptr != 0 && Handle.ptr != -1);
		D_RENDERER_DEVICE::GetDevice()->CreateSampler(this, Handle);
	}

}