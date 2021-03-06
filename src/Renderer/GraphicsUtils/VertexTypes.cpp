#include "../pch.hpp"
#include "VertexTypes.hpp"

#include <Utils/Assert.hpp>

namespace Darius::Renderer::GraphicsUtils::VertexTypes
{

	/////////////////////////////////////////////////
	/////////////// Position ////////////////////////
	/////////////////////////////////////////////////
	const D3D12_INPUT_ELEMENT_DESC VertexPosition::InputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	D_STATIC_ASSERT_M(sizeof(VertexPosition) == 12, "Vertex struct/layout mismatch");

	const D3D12_INPUT_LAYOUT_DESC VertexPosition::InputLayout =
	{
		VertexPosition::InputElements,
		VertexPosition::InputElementCount
	};

	/////////////////////////////////////////////////
	/////////////// Position Color //////////////////
	/////////////////////////////////////////////////
	const D3D12_INPUT_ELEMENT_DESC VertexPositionColor::InputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D_STATIC_ASSERT_M(sizeof(VertexPositionColor) == 28, "Vertex struct/layout mismatch");

	const D3D12_INPUT_LAYOUT_DESC VertexPositionColor::InputLayout =
	{
		VertexPositionColor::InputElements,
		VertexPositionColor::InputElementCount
	};

	/////////////////////////////////////////////////
	////////// Position Normal Tangent UV ///////////
	/////////////////////////////////////////////////
	const D3D12_INPUT_ELEMENT_DESC VertexPositionNormalTangentTexture::InputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D_STATIC_ASSERT_M(sizeof(VertexPositionNormalTangentTexture) == 44, "Vertex struct/layout mismatch");

	const D3D12_INPUT_LAYOUT_DESC VertexPositionNormalTangentTexture::InputLayout =
	{
		VertexPositionNormalTangentTexture::InputElements,
		VertexPositionNormalTangentTexture::InputElementCount
	};

}