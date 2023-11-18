#include "Renderer/pch.hpp"
#include "TerrainResource.hpp"

#include "Renderer/Rasterization/Renderer.hpp"
#include "Renderer/RendererManager.hpp"
#include "Renderer/Geometry/GeometryGenerator.hpp"

#include <Core/Containers/Vector.hpp>
#include <Graphics/GraphicsUtils/PipelineState.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Utils/Common.hpp>

#include <Libs/DirectXTex/DirectXTex/DirectXTex.h>

#ifdef _D_EDITOR
#include <ResourceManager/ResourceDragDropPayload.hpp>
#include <Libs/FontIcon/IconsFontAwesome6.h>
#include <imgui.h>
#endif

#include "TerrainResource.sgenerated.hpp"\

using namespace D_CONTAINERS;
using namespace D_GRAPHICS_UTILS;
using namespace D_RENDERER_GEOMETRY;
using namespace D_SERIALIZATION;

namespace Darius::Renderer
{

	// Internal
	ComputePSO				TerrainMeshGenerationCS(L"Terrain Mesh Generation CS");

	D_CH_RESOURCE_DEF(TerrainResource);

	struct TerrainParametersConstants
	{
		float HeightFactor;
	};

	TerrainResource::TerrainResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault) :
		Resource(uuid, path, name, id, isDefault),
		mHeightFactor(300.f),
		mHeightMap()
	{
		// Is pso setup yet?
		if (TerrainMeshGenerationCS.GetPipelineStateObject() == nullptr)
		{
			TerrainMeshGenerationCS.SetRootSignature(D_GRAPHICS::CommonRS);
			auto shader = D_GRAPHICS::GetShaderByName("TerrainMeshGeneratorCS");
			TerrainMeshGenerationCS.SetComputeShader(shader->GetBufferPointer(), shader->GetBufferSize());
			TerrainMeshGenerationCS.Finalize();
		}
	}

	void TerrainResource::WriteResourceToFile(D_SERIALIZATION::Json& j) const
	{
		Json json;

		D_H_SERIALIZE(HeightFactor);

		if (mHeightMap.IsValid())
		{
			json["HeightMap"] = D_CORE::ToString(mHeightMap->GetUuid());
		}

		D_FILE::WriteJsonFile(GetPath(), json);
	}

	void TerrainResource::ReadResourceFromFile(D_SERIALIZATION::Json const& j)
	{
		Json json;
		D_FILE::ReadJsonFile(GetPath(), json);

		D_H_DESERIALIZE(HeightFactor);

		if (json.contains("HeightMap"))
		{
			mHeightMap = D_RESOURCE::GetResourceSync<TextureResource>(D_CORE::FromString(json["HeightMap"]));

			if (mHeightMap.IsValid() && !mHeightMap->IsLoaded())
				D_RESOURCE_LOADER::LoadResourceAsync(mHeightMap.Get(), nullptr, true);

		}
	}

	bool TerrainResource::UploadToGpu()
	{

		auto renderType = D_RENDERER::GetActiveRendererType();

		switch (renderType)
		{
		case Darius::Renderer::RendererType::Rasterization:
			return InitRasterization();
		case Darius::Renderer::RendererType::RayTracing:
			return InitRayTracing();
		default:
			D_ASSERT_M(false, "Renderer not supported");
		}
		return false;
	}

	bool TerrainResource::InitRayTracing()
	{

		auto& context = D_GRAPHICS::ComputeContext::Begin(L"Initializing Ray Tracing Terrain Resource");

		if (mParametersConstantsGPU.GetGpuVirtualAddress() != D3D12_GPU_VIRTUAL_ADDRESS_NULL)
		{
			mParametersConstantsCPU.Destroy();
			mParametersConstantsGPU.Destroy();
		}

		if (mTexturesHeap.IsNull())
		{
			mTexturesHeap = D_RENDERER::AllocateTextureDescriptor();
		}

		if (mMesh.VertexDataGpu.GetGpuVirtualAddress() == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
		{
			D_PROFILING::ScopedTimer _prof(L"Creating Grid Mesh", context);

			auto name = GetName() + L"Terrain Mesh";

			auto grid = D_RENDERER_GEOMETRY_GENERATOR::CreateGrid(100, 100, 64 * 8, 64 * 8);

			DVector<D_RENDERER_VERTEX::VertexPositionNormalTangentTexture> vertices;
			DVector<std::uint32_t> indices;

			vertices.reserve(grid.Vertices.size());
			indices.reserve(grid.Indices32.size());

			// Filling vertex and index data
			for (int i = 0; i < grid.Vertices.size(); i++)
			{
				auto const& meshVertex = grid.Vertices[i];
				vertices.push_back(D_RENDERER_VERTEX::VertexPositionNormalTangentTexture(meshVertex.mPosition, D_MATH::Normalize(meshVertex.mNormal), meshVertex.mTangent, meshVertex.mTexC));
			}

			auto const& indicesSrc = grid.Indices32;
			for (int i = 0; i < indicesSrc.size(); i++)
			{
				indices.push_back(indicesSrc[i]);
			}

			mMesh.mDraw.clear();
			Mesh::Draw gpuDraw = { (UINT)indices.size(), 0, 0 };
			mMesh.mDraw.push_back(gpuDraw);

			mMesh.mNumTotalVertices = (UINT)vertices.size();
			mMesh.mNumTotalIndices = (UINT)indices.size();

			mMesh.Name = GetName();

			// Create vertex buffer
			mMesh.VertexDataGpu.Create(mMesh.Name + L" Vertex Buffer", (UINT)vertices.size(), sizeof(D_RENDERER_VERTEX::VertexPositionNormalTangentTexture), vertices.data());

			// Create index buffer
			mMesh.IndexDataGpu.Create(mMesh.Name + L" Index Buffer", (UINT)indices.size(), sizeof(std::uint32_t), indices.data());
		}

		if (!mHeightMap.IsValid())
			mHeightMap = D_RESOURCE::GetResourceSync<TextureResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DBlackOpaque));

		if (!mHeightMap->GetDefault() && D_H_ENSURE_FILE(mHeightMap->GetPath()))
		{
			D_PROFILING::ScopedTimer _prof(L"Calculating Terrain Normal Map");

			DirectX::TexMetadata info;
			DirectX::ScratchImage heightImage;
			DirectX::ScratchImage normalImage;

			{
				D_PROFILING::ScopedTimer _prof(L"Reading Height Map");

				DirectX::LoadFromDDSFile(mHeightMap->GetPath().c_str(), DirectX::DDS_FLAGS_NONE, &info, heightImage);
			}

			DirectX::ComputeNormalMap(*heightImage.GetImage(0, 0, 0), DirectX::CNMAP_CHANNEL_RED, mHeightFactor, DXGI_FORMAT_R8G8B8A8_UNORM, normalImage);

			auto normalImg = normalImage.GetImage(0, 0, 0);
			mGeneratedNormalMap.Create2D(normalImg->rowPitch, info.width, info.height, normalImage.GetMetadata().format, normalImage.GetPixels());

			heightImage.Release();
			normalImage.Release();

		}
		else
		{
			uint32_t color = 0x00FF0000;
			mGeneratedNormalMap.Create2D(4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &color);
		}

		{
			D_PROFILING::ScopedTimer _prof(L"Applying Terrain Height", context);

			context.TransitionResource(mMesh.VertexDataGpu, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			context.SetPipelineState(TerrainMeshGenerationCS);
			context.SetRootSignature(D_GRAPHICS::CommonRS);

			context.SetConstants(0, mHeightFactor);
			context.SetDynamicDescriptor(1, 0, mHeightMap->GetTextureData()->GetSRV());
			context.SetDynamicDescriptor(1, 1, mGeneratedNormalMap.GetSRV());
			context.SetDynamicDescriptor(2, 0, mMesh.VertexDataGpu.GetUAV());
			context.Dispatch2D(mMesh.mNumTotalVertices, 1u, 8u, 1u);

			context.TransitionResource(mMesh.VertexDataGpu, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}

		context.Finish(true);
		return true;
	}

	bool TerrainResource::InitRasterization()
	{
		// Creating buffers
		if (mParametersConstantsGPU.GetGpuVirtualAddress() == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
		{
			// Releasing raytracing stuff
			mMesh = {};

			// Initializing patameters constants buffers
			mParametersConstantsCPU.Create(L"Terrain Parameter Constants Buffer: " + GetName(), sizeof(TerrainParametersConstants), D_GRAPHICS_DEVICE::gNumFrameResources);
			mParametersConstantsGPU.Create(L"Terrain Parameter Constants Buffer: " + GetName(), 1, sizeof(TerrainParametersConstants));

			mTexturesHeap = D_RENDERER::AllocateTextureDescriptor(1);
		}

		if (!mHeightMap.IsValid())
			mHeightMap = D_RESOURCE::GetResourceSync<TextureResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DBlackOpaque));

		UINT destCount = 1;
		UINT srcCount[] = { 1 };
		D3D12_CPU_DESCRIPTOR_HANDLE initializeTextures[1]
		{
			mHeightMap->GetTextureData()->GetSRV()
		};
		D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptors(1, &mTexturesHeap, &destCount, destCount, initializeTextures, srcCount, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Updating parameter constants
		UINT frameResIndex = D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex();
		auto paramCB = reinterpret_cast<TerrainParametersConstants*>(mParametersConstantsCPU.MapInstance(frameResIndex));
		paramCB->HeightFactor = mHeightFactor;
		mParametersConstantsCPU.Unmap();

		// Uploading
		auto& context = D_GRAPHICS::CommandContext::Begin(L"Terrain Params Uploadier");
		context.TransitionResource(mParametersConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
		context.CopyBufferRegion(mParametersConstantsGPU, 0, mParametersConstantsCPU, frameResIndex * mParametersConstantsCPU.GetBufferSize(), mParametersConstantsCPU.GetBufferSize());
		context.TransitionResource(mParametersConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);
		context.Finish(true);

		return true;
	}

#ifdef _D_EDITOR
	bool TerrainResource::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();


		{
			D_H_DETAILS_DRAW_PROPERTY("Height Map");
			D_H_RESOURCE_SELECTION_DRAW(TextureResource, mHeightMap, "Select Height Map", SetHeightMap);
		}

		{
			D_H_DETAILS_DRAW_PROPERTY("Heght Factor");

			float heightFactor = GetHeightFactor();
			if (ImGui::DragFloat("##HeightFactor", &heightFactor))
			{
				SetHeightFactor(heightFactor);
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		if (valueChanged)
		{
			MakeDiskDirty();
			MakeGpuDirty();
		}

		return valueChanged;
	}
#endif

	void TerrainResource::SetHeightMap(TextureResource* heightMap)
	{
		if (mHeightMap == heightMap)
			return;

		mHeightMap = heightMap;

		if (mHeightMap.IsValid() && !mHeightMap->IsLoaded())
			D_RESOURCE_LOADER::LoadResourceAsync(heightMap, nullptr, true);

		MakeDiskDirty();
		MakeGpuDirty();

		SignalChange();
	}

	void TerrainResource::SetHeightFactor(float value)
	{
		if (value == mHeightFactor)
			return;

		mHeightFactor = value;

		MakeDiskDirty();
		MakeGpuDirty();

		SignalChange();
	}

}
