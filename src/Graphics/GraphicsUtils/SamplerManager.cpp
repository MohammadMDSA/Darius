#include "Graphics/pch.hpp"
#include "SamplerManager.hpp"

#include "Graphics/GraphicsCore.hpp"
#include "Graphics/GraphicsDeviceManager.hpp"

#include <Core/Hash.hpp>
#include <Core/Serialization/Json.hpp>
#include <Core/Serialization/TypeSerializer.hpp>
#include <Utils/Assert.hpp>

#include <map>

using namespace std;
using namespace D_SERIALIZATION;

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
		s_SamplerCache.emplace(hashValue, Handle);

		D_GRAPHICS_DEVICE::GetDevice()->CreateSampler(this, Handle);
		return Handle;
	}

	void SamplerDesc::CreateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE Handle)
	{
		D_ASSERT(Handle.ptr != 0 && Handle.ptr != -1);

		size_t hashValue = D_CORE::HashState(this);

		D_GRAPHICS_DEVICE::GetDevice()->CreateSampler(this, Handle);
	}

	void SamplerDesc::Serialize(Json& json) const
	{
		D_MATH::Color border(BorderColor[0], BorderColor[1], BorderColor[2], BorderColor[3]);

		json.clear();
		json["Filter"] = (uint32_t)Filter;
		json["AddressU"] = (uint32_t)AddressU;
		json["AddressV"] = (uint32_t)AddressV;
		json["AddressW"] = (uint32_t)AddressW;
		json["MipLODBias"] = MipLODBias;
		json["MaxAnisotropy"] = (uint32_t)MaxAnisotropy;
		json["ComparisonFunc"] = (uint32_t)ComparisonFunc;
		D_SERIALIZATION::Serialize(border, json["BorderColor"]);
		json["MinLOD"] = (float)MinLOD;
		json["MaxLOD"] = (float)MaxLOD;
	}

	void SamplerDesc::Deserialize(Json const& json)
	{
		D_MATH::Color border;
		Filter = (D3D12_FILTER)json["Filter"].get<uint32_t>();
		AddressU = (D3D12_TEXTURE_ADDRESS_MODE)json["AddressU"].get<uint32_t>();
		AddressV = (D3D12_TEXTURE_ADDRESS_MODE)json["AddressV"].get<uint32_t>();
		AddressW = (D3D12_TEXTURE_ADDRESS_MODE)json["AddressW"].get<uint32_t>();
		MipLODBias = json["MipLODBias"];
		MaxAnisotropy = json["MaxAnisotropy"].get<uint32_t>();
		ComparisonFunc = (D3D12_COMPARISON_FUNC)json["ComparisonFunc"].get<uint32_t>();
		D_SERIALIZATION::Deserialize(border, json["BorderColor"]);
		BorderColor[0] = border.GetR();
		BorderColor[1] = border.GetG();
		BorderColor[2] = border.GetB();
		BorderColor[3] = border.GetA();
		MinLOD = json["MinLOD"];
		MaxLOD = json["MaxLOD"];
	}
}