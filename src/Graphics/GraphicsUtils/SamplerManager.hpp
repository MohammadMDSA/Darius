#pragma once

#include <Core/Serialization/Json.hpp>
#include <Math/Color.hpp>

#ifndef D_GRAPHICS_UTILS
#define D_GRAPHICS_UTILS Darius::Graphics::Utils
#endif

namespace Darius::Graphics::Utils
{
	class SamplerDesc : public D3D12_SAMPLER_DESC
	{
	public:
		// These defaults match the default values for HLSL-defined root
		// signature static samplers.  So not overriding them here means
		// you can safely not define them in HLSL.
		SamplerDesc()
		{
			Filter = D3D12_FILTER_ANISOTROPIC;
			AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			MipLODBias = 0.0f;
			MaxAnisotropy = 16;
			ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			BorderColor[0] = 1.0f;
			BorderColor[1] = 1.0f;
			BorderColor[2] = 1.0f;
			BorderColor[3] = 1.0f;
			MinLOD = 0.0f;
			MaxLOD = D3D12_FLOAT32_MAX;
		}

		void SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE AddressMode)
		{
			AddressU = AddressMode;
			AddressV = AddressMode;
			AddressW = AddressMode;
		}

		void SetBorderColor(D_MATH::Color Border)
		{
			BorderColor[0] = Border.GetR();
			BorderColor[1] = Border.GetG();
			BorderColor[2] = Border.GetB();
			BorderColor[3] = Border.GetA();
		}

		bool operator ==(SamplerDesc const& other) const
		{
			return
				this->Filter == other.Filter &&
				this->AddressU == other.AddressU &&
				this->AddressV == other.AddressV &&
				this->AddressW == other.AddressW &&
				this->MipLODBias == other.MipLODBias &&
				this->MaxAnisotropy == other.MaxAnisotropy &&
				this->ComparisonFunc == other.ComparisonFunc &&
				this->BorderColor[0] == other.BorderColor[0] &&
				this->BorderColor[1] == other.BorderColor[1] &&
				this->BorderColor[2] == other.BorderColor[2] &&
				this->BorderColor[3] == other.BorderColor[3] &&
				this->MinLOD == other.MinLOD &&
				this->MaxLOD == other.MaxLOD;
		}

		// Allocate new descriptor as needed; return handle to existing descriptor when possible
		D3D12_CPU_DESCRIPTOR_HANDLE CreateDescriptor(void);

		// Create descriptor in place (no deduplication).  Handle must be preallocated
		void CreateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE Handle);

		void Serialize(Darius::Core::Serialization::Json& json) const;
		void Deserialize(Darius::Core::Serialization::Json const& json);
	};
}