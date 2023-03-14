#pragma once

#include <Math/Vector.hpp>

#define D_GRAPHICS_VERTEX Darius::Graphics::Utils::VertexTypes

namespace Darius::Graphics::Utils::VertexTypes
{
	interface IVertexType {};

	struct VertexPosition : public IVertexType
	{
		VertexPosition() = default;

		VertexPosition(const VertexPosition&) = default;
		VertexPosition& operator=(const VertexPosition&) = default;

		VertexPosition(VertexPosition&&) = default;
		VertexPosition& operator=(VertexPosition&&) = default;

		VertexPosition(D_MATH::Vector3 const& position) noexcept :
			mPosition(position)
		{
		}

		VertexPosition(D_MATH::Vector3 position) noexcept :
			mPosition(position)
		{
		}

		DirectX::XMFLOAT3						mPosition;

		static const D3D12_INPUT_LAYOUT_DESC	InputLayout;

	private:
		static constexpr unsigned int			InputElementCount = 1;
		static const D3D12_INPUT_ELEMENT_DESC	InputElements[InputElementCount];
	};

	struct VertexPositionSkinned : public IVertexType
	{
		VertexPositionSkinned() = default;

		VertexPositionSkinned(const VertexPositionSkinned&) = default;
		VertexPositionSkinned& operator=(const VertexPositionSkinned&) = default;

		VertexPositionSkinned(VertexPositionSkinned&&) = default;
		VertexPositionSkinned& operator=(VertexPositionSkinned&&) = default;

		VertexPositionSkinned(D_MATH::Vector3 const& position,
			DirectX::XMUINT4 const& blendIndices = { 0, 0, 0, 0 }, DirectX::XMFLOAT4 blendWeights = { 0.f, 0.f, 0.f, 0.f }) noexcept :
			mPosition(position),
			mBlendIndices(blendIndices),
			mBlendWeights(blendWeights)
		{
		}

		VertexPositionSkinned(D_MATH::Vector3 position,
			DirectX::XMUINT4 const& blendIndices = { 0, 0, 0, 0 }, DirectX::XMFLOAT4 blendWeights = { 0.f, 0.f, 0.f, 0.f }) noexcept :
			mPosition(position),
			mBlendIndices(blendIndices),
			mBlendWeights(blendWeights)
		{
		}

		DirectX::XMFLOAT3						mPosition;
		DirectX::XMUINT4						mBlendIndices;
		DirectX::XMFLOAT4						mBlendWeights;

		static const D3D12_INPUT_LAYOUT_DESC	InputLayout;

	private:
		static constexpr unsigned int			InputElementCount = 3;
		static const D3D12_INPUT_ELEMENT_DESC	InputElements[InputElementCount];
	};

	struct VertexPositionTexture : public IVertexType
	{
		VertexPositionTexture() = default;

		VertexPositionTexture(const VertexPositionTexture&) = default;
		VertexPositionTexture& operator=(const VertexPositionTexture&) = default;

		VertexPositionTexture(VertexPositionTexture&&) = default;
		VertexPositionTexture& operator=(VertexPositionTexture&&) = default;

		VertexPositionTexture(D_MATH::Vector3 const& position, float u, float v) noexcept :
			mPosition(position),
			mUV(u, v)
		{
		}

		VertexPositionTexture(D_MATH::Vector3 position, float u, float v) noexcept :
			mPosition(position),
			mUV(u, v)
		{
		}

		DirectX::XMFLOAT3						mPosition;
		DirectX::XMFLOAT2						mUV;

		static const D3D12_INPUT_LAYOUT_DESC	InputLayout;

	private:
		static constexpr unsigned int			InputElementCount = 2;
		static const D3D12_INPUT_ELEMENT_DESC	InputElements[InputElementCount];
	};

	struct VertexPositionTextureSkinned : public IVertexType
	{
		VertexPositionTextureSkinned() = default;

		VertexPositionTextureSkinned(const VertexPositionTextureSkinned&) = default;
		VertexPositionTextureSkinned& operator=(const VertexPositionTextureSkinned&) = default;

		VertexPositionTextureSkinned(VertexPositionTextureSkinned&&) = default;
		VertexPositionTextureSkinned& operator=(VertexPositionTextureSkinned&&) = default;

		VertexPositionTextureSkinned(D_MATH::Vector3 const& position, float u, float v,
			DirectX::XMUINT4 const& blendIndices = { 0, 0, 0, 0 }, DirectX::XMFLOAT4 blendWeights = { 0.f, 0.f, 0.f, 0.f }) noexcept :
			mPosition(position),
			mUV(u, v),
			mBlendIndices(blendIndices),
			mBlendWeights(blendWeights)
		{
		}

		VertexPositionTextureSkinned(D_MATH::Vector3 position, float u, float v,
			DirectX::XMUINT4 const& blendIndices = { 0, 0, 0, 0 }, DirectX::XMFLOAT4 blendWeights = { 0.f, 0.f, 0.f, 0.f }) noexcept :
			mPosition(position),
			mUV(u, v),
			mBlendIndices(blendIndices),
			mBlendWeights(blendWeights)
		{
		}

		DirectX::XMFLOAT3						mPosition;
		DirectX::XMFLOAT2						mUV;
		DirectX::XMUINT4						mBlendIndices;
		DirectX::XMFLOAT4						mBlendWeights;

		static const D3D12_INPUT_LAYOUT_DESC	InputLayout;

	private:
		static constexpr unsigned int			InputElementCount = 4;
		static const D3D12_INPUT_ELEMENT_DESC	InputElements[InputElementCount];
	};

	struct VertexPositionNormal : public IVertexType
	{
		VertexPositionNormal() = default;

		VertexPositionNormal(const VertexPositionNormal&) = default;
		VertexPositionNormal& operator=(const VertexPositionNormal&) = default;

		VertexPositionNormal(VertexPositionNormal&&) = default;
		VertexPositionNormal& operator=(VertexPositionNormal&&) = default;

		VertexPositionNormal(D_MATH::Vector3 const& position, D_MATH::Vector3 const& normal) noexcept :
			mPosition(position),
			mNormal(normal)
		{
		}

		DirectX::XMFLOAT3						mPosition;
		DirectX::XMFLOAT3						mNormal;

		static const D3D12_INPUT_LAYOUT_DESC	InputLayout;

	private:
		static constexpr unsigned int			InputElementCount = 2;
		static const D3D12_INPUT_ELEMENT_DESC	InputElements[InputElementCount];
	};

	struct VertexPositionNormalTexture : public IVertexType
	{
		VertexPositionNormalTexture() = default;

		VertexPositionNormalTexture(const VertexPositionNormalTexture&) = default;
		VertexPositionNormalTexture& operator=(const VertexPositionNormalTexture&) = default;

		VertexPositionNormalTexture(VertexPositionNormalTexture&&) = default;
		VertexPositionNormalTexture& operator=(VertexPositionNormalTexture&&) = default;

		VertexPositionNormalTexture(D_MATH::Vector3 const& position, D_MATH::Vector3 const& normal, DirectX::XMFLOAT2 const& tex) noexcept :
			mPosition(position),
			mNormal(normal),
			mTexCoord(tex)
		{
		}

		DirectX::XMFLOAT3						mPosition;
		DirectX::XMFLOAT3						mNormal;
		DirectX::XMFLOAT2						mTexCoord;

		static const D3D12_INPUT_LAYOUT_DESC	InputLayout;

	private:
		static constexpr unsigned int			InputElementCount = 3;
		static const D3D12_INPUT_ELEMENT_DESC	InputElements[InputElementCount];
	};

	struct VertexPositionNormalTextureSkinned : public IVertexType
	{
		VertexPositionNormalTextureSkinned() = default;

		VertexPositionNormalTextureSkinned(const VertexPositionNormalTextureSkinned&) = default;
		VertexPositionNormalTextureSkinned& operator=(const VertexPositionNormalTextureSkinned&) = default;

		VertexPositionNormalTextureSkinned(VertexPositionNormalTextureSkinned&&) = default;
		VertexPositionNormalTextureSkinned& operator=(VertexPositionNormalTextureSkinned&&) = default;

		VertexPositionNormalTextureSkinned(D_MATH::Vector3 const& position, D_MATH::Vector3 const& normal, DirectX::XMFLOAT2 const& tex, DirectX::XMUINT4 const& blendIndices, DirectX::XMFLOAT4 blendWeights) noexcept :
			mPosition(position),
			mNormal(normal),
			mTexCoord(tex),
			mBlendIndices(blendIndices),
			mBlendWeights(blendWeights)
		{
		}

		DirectX::XMFLOAT3						mPosition;
		DirectX::XMFLOAT3						mNormal;
		DirectX::XMFLOAT2						mTexCoord;
		DirectX::XMUINT4						mBlendIndices;
		DirectX::XMFLOAT4						mBlendWeights;

		static const D3D12_INPUT_LAYOUT_DESC	InputLayout;

	private:
		static constexpr unsigned int			InputElementCount = 5;
		static const D3D12_INPUT_ELEMENT_DESC	InputElements[InputElementCount];
	};

	struct VertexPositionColor : public IVertexType
	{
		VertexPositionColor() = default;

		VertexPositionColor(const VertexPositionColor&) = default;
		VertexPositionColor& operator=(const VertexPositionColor&) = default;

		VertexPositionColor(VertexPositionColor&&) = default;
		VertexPositionColor& operator=(VertexPositionColor&&) = default;

		VertexPositionColor(D_MATH::Vector3 const& position, D_MATH::Vector4 const& color) noexcept :
			mPosition(position),
			mColor(color)
		{
		}
		
		DirectX::XMFLOAT3						mPosition;
		DirectX::XMFLOAT4						mColor;

		static const D3D12_INPUT_LAYOUT_DESC	InputLayout;

	private:
		static constexpr unsigned int			InputElementCount = 2;
		static const D3D12_INPUT_ELEMENT_DESC	InputElements[InputElementCount];
	};

	struct VertexPositionNormalTangentTexture : public IVertexType
	{
		VertexPositionNormalTangentTexture() = default;

		VertexPositionNormalTangentTexture(const VertexPositionNormalTangentTexture&) = default;
		VertexPositionNormalTangentTexture& operator=(const VertexPositionNormalTangentTexture&) = default;

		VertexPositionNormalTangentTexture(VertexPositionNormalTangentTexture&&) = default;
		VertexPositionNormalTangentTexture& operator=(VertexPositionNormalTangentTexture&&) = default;

		VertexPositionNormalTangentTexture(D_MATH::Vector3 const& pos, D_MATH::Vector3 const& norm, D_MATH::Vector4 const& tang, DirectX::XMFLOAT2 const& uv) :
			mPosition(pos),
			mNormal(norm),
			mTangent(tang),
			mTexC(uv)
		{
		}

		VertexPositionNormalTangentTexture(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz, float tw,
			float u, float v) :
			mPosition(px, py, pz),
			mNormal(nx, ny, nz),
			mTangent(tx, ty, tz, tw),
			mTexC(u, v)
		{
		}

		DirectX::XMFLOAT3						mPosition;
		DirectX::XMFLOAT3						mNormal;
		DirectX::XMFLOAT4						mTangent;
		DirectX::XMFLOAT2						mTexC;

		static const D3D12_INPUT_LAYOUT_DESC	InputLayout;

	private:
		static constexpr unsigned int			InputElementCount = 4;
		static const D3D12_INPUT_ELEMENT_DESC	InputElements[InputElementCount];
	};


	struct VertexPositionNormalTangentTextureSkinned : public IVertexType
	{
		VertexPositionNormalTangentTextureSkinned() = default;

		VertexPositionNormalTangentTextureSkinned(const VertexPositionNormalTangentTextureSkinned&) = default;
		VertexPositionNormalTangentTextureSkinned& operator=(const VertexPositionNormalTangentTextureSkinned&) = default;

		VertexPositionNormalTangentTextureSkinned(VertexPositionNormalTangentTextureSkinned&&) = default;
		VertexPositionNormalTangentTextureSkinned& operator=(VertexPositionNormalTangentTextureSkinned&&) = default;

		VertexPositionNormalTangentTextureSkinned(D_MATH::Vector3 const& pos, D_MATH::Vector3 const& norm, D_MATH::Vector4 const& tang, DirectX::XMFLOAT2 const& uv,
			DirectX::XMUINT4 const& blendIndices = { 0, 0, 0, 0 }, DirectX::XMFLOAT4 blendWeights = { 0.f, 0.f, 0.f, 0.f }) :
			mPosition(pos),
			mNormal(norm),
			mTangent(tang),
			mTexC(uv),
			mBlendIndices(blendIndices),
			mBlendWeights(blendWeights)
		{
		}

		VertexPositionNormalTangentTextureSkinned(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz, float tw,
			float u, float v,
			DirectX::XMUINT4 const& blendIndices = { 0, 0, 0, 0 }, DirectX::XMFLOAT4 blendWeights = { 0.f, 0.f, 0.f, 0.f }) :
			mPosition(px, py, pz),
			mNormal(nx, ny, nz),
			mTangent(tx, ty, tz, tw),
			mTexC(u, v),
			mBlendIndices(blendIndices),
			mBlendWeights(blendWeights)
		{
		}

		DirectX::XMFLOAT3						mPosition;
		DirectX::XMFLOAT3						mNormal;
		DirectX::XMFLOAT4						mTangent;
		DirectX::XMFLOAT2						mTexC;
		DirectX::XMUINT4						mBlendIndices;
		DirectX::XMFLOAT4						mBlendWeights;

		static const D3D12_INPUT_LAYOUT_DESC	InputLayout;

	private:
		static constexpr unsigned int			InputElementCount = 6;
		static const D3D12_INPUT_ELEMENT_DESC	InputElements[InputElementCount];
	};
}