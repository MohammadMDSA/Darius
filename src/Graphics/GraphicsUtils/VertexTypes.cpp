#include "../pch.hpp"
#include "VertexTypes.hpp"

#include <Utils/Assert.hpp>

namespace Darius::Graphics::Utils::VertexTypes
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
	/////////////// Position Skinned ////////////////
	/////////////////////////////////////////////////
	const D3D12_INPUT_ELEMENT_DESC VertexPositionSkinned::InputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D_STATIC_ASSERT_M(sizeof(VertexPositionSkinned) == 44, "Vertex struct/layout mismatch");

	const D3D12_INPUT_LAYOUT_DESC VertexPositionSkinned::InputLayout =
	{
		VertexPositionSkinned::InputElements,
		VertexPositionSkinned::InputElementCount
	};

	/////////////////////////////////////////////////
	/////////////// Position Texture ////////////////
	/////////////////////////////////////////////////
	const D3D12_INPUT_ELEMENT_DESC VertexPositionTexture::InputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D_STATIC_ASSERT_M(sizeof(VertexPositionTexture) == 20, "Vertex struct/layout mismatch");

	const D3D12_INPUT_LAYOUT_DESC VertexPositionTexture::InputLayout =
	{
		VertexPositionTexture::InputElements,
		VertexPositionTexture::InputElementCount
	};

	/////////////////////////////////////////////////
	/////////// Position Texture Skinned ////////////
	/////////////////////////////////////////////////
	const D3D12_INPUT_ELEMENT_DESC VertexPositionTextureSkinned::InputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D_STATIC_ASSERT_M(sizeof(VertexPositionTextureSkinned) == 52, "Vertex struct/layout mismatch");

	const D3D12_INPUT_LAYOUT_DESC VertexPositionTextureSkinned::InputLayout =
	{
		VertexPositionTextureSkinned::InputElements,
		VertexPositionTextureSkinned::InputElementCount
	};

	/////////////////////////////////////////////////
	/////////////// Position Normal //////////////////
	/////////////////////////////////////////////////
	const D3D12_INPUT_ELEMENT_DESC VertexPositionNormal::InputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D_STATIC_ASSERT_M(sizeof(VertexPositionNormal) == 24, "Vertex struct/layout mismatch");

	const D3D12_INPUT_LAYOUT_DESC VertexPositionNormal::InputLayout =
	{
		VertexPositionNormal::InputElements,
		VertexPositionNormal::InputElementCount
	};


	/////////////////////////////////////////////////
	////////// Position Normal Texture //////////////
	/////////////////////////////////////////////////
	const D3D12_INPUT_ELEMENT_DESC VertexPositionNormalTexture::InputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D_STATIC_ASSERT_M(sizeof(VertexPositionNormalTexture) == 32, "Vertex struct/layout mismatch");

	const D3D12_INPUT_LAYOUT_DESC VertexPositionNormalTexture::InputLayout =
	{
		VertexPositionNormalTexture::InputElements,
		VertexPositionNormalTexture::InputElementCount
	};


	/////////////////////////////////////////////////
	////// Position Normal Texture Skinned //////////
	/////////////////////////////////////////////////
	const D3D12_INPUT_ELEMENT_DESC VertexPositionNormalTextureSkinned::InputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D_STATIC_ASSERT_M(sizeof(VertexPositionNormalTextureSkinned) == 64, "Vertex struct/layout mismatch");

	const D3D12_INPUT_LAYOUT_DESC VertexPositionNormalTextureSkinned::InputLayout =
	{
		VertexPositionNormalTextureSkinned::InputElements,
		VertexPositionNormalTextureSkinned::InputElementCount
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
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D_STATIC_ASSERT_M(sizeof(VertexPositionNormalTangentTexture) == 48, "Vertex struct/layout mismatch");

	const D3D12_INPUT_LAYOUT_DESC VertexPositionNormalTangentTexture::InputLayout =
	{
		VertexPositionNormalTangentTexture::InputElements,
		VertexPositionNormalTangentTexture::InputElementCount
	};

	/////////////////////////////////////////////////
	/////// Position Normal Tangent UV Skined ///////
	/////////////////////////////////////////////////
	const D3D12_INPUT_ELEMENT_DESC VertexPositionNormalTangentTextureSkinned::InputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D_STATIC_ASSERT_M(sizeof(VertexPositionNormalTangentTextureSkinned) == 80, "Vertex struct/layout mismatch");

	const D3D12_INPUT_LAYOUT_DESC VertexPositionNormalTangentTextureSkinned::InputLayout =
	{
		VertexPositionNormalTangentTextureSkinned::InputElements,
		VertexPositionNormalTangentTextureSkinned::InputElementCount
	};

}