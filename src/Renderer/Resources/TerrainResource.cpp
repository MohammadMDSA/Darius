#include "Renderer/pch.hpp"
#include "TerrainResource.hpp"

#include "Renderer/Rasterization/Renderer.hpp"
#include "Renderer/RendererManager.hpp"

#include <ResourceManager/ResourceManager.hpp>
#include <Utils/Common.hpp>

#ifdef _D_EDITOR
#include <ResourceManager/ResourceDragDropPayload.hpp>
#include <Libs/FontIcon/IconsFontAwesome6.h>
#include <imgui.h>
#endif

#include "TerrainResource.sgenerated.hpp"\

using namespace D_SERIALIZATION;

namespace Darius::Renderer
{

	D_CH_RESOURCE_DEF(TerrainResource);

	struct TerrainParametersConstants
	{
		float HeightFactor;
	};

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
			mHeightMap = D_RESOURCE::GetResource<TextureResource>(D_CORE::FromString(json["HeightMap"]), GetAsCountedOwner());
		}
	}

	bool TerrainResource::UploadToGpu()
	{
		// Creating buffers
		if (mParametersConstantsGPU.GetGpuVirtualAddress() == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
		{
			// Initializing patameters constants buffers
			mParametersConstantsCPU.Create(L"Terrain Parameter Constants Buffer: " + GetName(), sizeof(TerrainParametersConstants));
			mParametersConstantsGPU.Create(L"Terrain Parameter Constants Buffer: " + GetName(), 1, sizeof(TerrainParametersConstants));

			mTexturesHeap = D_RENDERER::AllocateTextureDescriptor(1);
		}

		if (!mHeightMap.IsValid())
			mHeightMap = D_RESOURCE::GetResource<TextureResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DBlackOpaque), *this);

		UINT destCount = 1;
		UINT srcCount[] = { 1 };
		D3D12_CPU_DESCRIPTOR_HANDLE initializeTextures[1]
		{
			mHeightMap->GetTextureData()->GetSRV()
		};
		D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptors(1, &mTexturesHeap, &destCount, destCount, initializeTextures, srcCount, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Updating parameter constants
		auto paramCB = reinterpret_cast<TerrainParametersConstants*>(mParametersConstantsCPU.Map());
		paramCB->HeightFactor = mHeightFactor;
		mParametersConstantsCPU.Unmap();

		// Uploading
		auto& context = D_GRAPHICS::CommandContext::Begin(L"Terrain Params Uploadier");
		context.TransitionResource(mParametersConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
		context.CopyBufferRegion(mParametersConstantsGPU, 0, mParametersConstantsCPU, 0, mParametersConstantsCPU.GetBufferSize());
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

	void TerrainResource::SetHeightMap(D_RESOURCE::ResourceHandle handle)
	{
		mHeightMap = D_RESOURCE::GetResource<TextureResource>(handle, *this);
		MakeDiskDirty();
		MakeGpuDirty();
	}

	bool TerrainResource::IsDirtyGPU() const
	{
		return Resource::IsDirtyGPU() ||
			(mHeightMap.IsValid() && mHeightMap->IsDirtyGPU());
	}

}
