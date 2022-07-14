#pragma once

#include <Math/Vector.hpp>

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

		Vector3									mPosition;

		static const D3D12_INPUT_LAYOUT_DESC	InputLayout;

	private:
		static constexpr unsigned int			InputElementCount = 1;
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

		VertexPositionColor(Vector3 position, Vector4 color) noexcept :
			mPosition(position),
			mColor(color)
		{
		}



		Vector3									mPosition;
		Vector4									mColor;

		static const D3D12_INPUT_LAYOUT_DESC	InputLayout;

	private:
		static constexpr unsigned int			InputElementCount = 2;
		static const D3D12_INPUT_ELEMENT_DESC	InputElements[InputElementCount];
	};
}