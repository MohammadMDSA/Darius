#pragma once

#include <Math/Vector.hpp>

#define D_RENDERER_VERTEX Darius::Renderer::GraphicsUtils::VertexTypes

using namespace Darius::Math;

namespace Darius::Renderer::GraphicsUtils::VertexTypes
{
	interface IVertexType {};

	struct VertexPosition : public IVertexType
	{
		VertexPosition() = default;

		VertexPosition(const VertexPosition&) = default;
		VertexPosition& operator=(const VertexPosition&) = default;

		VertexPosition(VertexPosition&&) = default;
		VertexPosition& operator=(VertexPosition&&) = default;
		
		VertexPosition(Vector3 const& position) noexcept :
			mPosition(position)
		{
		}

		VertexPosition(Vector3 position) noexcept :
			mPosition(position)
		{
		}

		XMFLOAT3								mPosition;

		static const D3D12_INPUT_LAYOUT_DESC	InputLayout;

	private:
		static constexpr unsigned int			InputElementCount = 1;
		static const D3D12_INPUT_ELEMENT_DESC	InputElements[InputElementCount];
	};

	struct VertexPositionNormal : public IVertexType
	{
		VertexPositionNormal() = default;

		VertexPositionNormal(const VertexPositionNormal&) = default;
		VertexPositionNormal& operator=(const VertexPositionNormal&) = default;

		VertexPositionNormal(VertexPositionNormal&&) = default;
		VertexPositionNormal& operator=(VertexPositionNormal&&) = default;

		VertexPositionNormal(Vector3 const& position, Vector3 const& normal) noexcept :
			mPosition(position),
			mNormal(normal)
		{
		}

		XMFLOAT3								mPosition;
		XMFLOAT3								mNormal;

		static const D3D12_INPUT_LAYOUT_DESC	InputLayout;

	private:
		static constexpr unsigned int			InputElementCount = 2;
		static const D3D12_INPUT_ELEMENT_DESC	InputElements[InputElementCount];
	};

	struct VertexPositionColor : public IVertexType
	{
		VertexPositionColor() = default;

		VertexPositionColor(const VertexPositionColor&) = default;
		VertexPositionColor& operator=(const VertexPositionColor&) = default;

		VertexPositionColor(VertexPositionColor&&) = default;
		VertexPositionColor& operator=(VertexPositionColor&&) = default;

		VertexPositionColor(Vector3 const& position, Vector4 const& color) noexcept :
			mPosition(position),
			mColor(color)
		{
		}
		
		XMFLOAT3								mPosition;
		XMFLOAT4								mColor;

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

		VertexPositionNormalTangentTexture(Vector3 const& pos, Vector3 const& norm, Vector3 const& tang, XMFLOAT2 const& uv) :
			mPosition(pos),
			mNormal(norm),
			mTangent(tang),
			mTexC(uv)
		{
		}

		VertexPositionNormalTangentTexture(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v) :
			mPosition(px, py, pz),
			mNormal(nx, ny, nz),
			mTangent(tx, ty, tz),
			mTexC(u, v)
		{
		}

		XMFLOAT3								mPosition;
		XMFLOAT3								mNormal;
		XMFLOAT3								mTangent;
		XMFLOAT2								mTexC;

		static const D3D12_INPUT_LAYOUT_DESC	InputLayout;

	private:
		static constexpr unsigned int			InputElementCount = 4;
		static const D3D12_INPUT_ELEMENT_DESC	InputElements[InputElementCount];
	};
}