#include "pch.hpp"
#include "MaterialResource.hpp"

#include <Core/Serialization/Json.hpp>
#include <Renderer/RenderDeviceManager.hpp>
#include <Utils/Common.hpp>

#include <nlohmann/json.hpp>

#include <fstream>

using namespace D_CORE;

namespace Darius::ResourceManager
{
	bool MaterialResource::SuppoertsExtension(std::wstring ext)
	{
		if (ext == L".mat")
			return true;
		return false;
	}

	void MaterialResource::WriteResourceToFile() const
	{
		float* defalb = (float*)&mMaterial.DifuseAlbedo;
		float* fren = (float*)&mMaterial.FresnelR0;
		Json data = {
			{ "DefuseAlbedo", std::vector<float>(defalb, defalb + 4)},
			{ "FresnelR0", std::vector<float>(fren, fren + 3) },
			{ "Roughness", mMaterial.Roughness }
		};

		std::ofstream os(GetPath());
		os << data;
	}

	void MaterialResource::ReadResourceFromFile()
	{

		Json data;
		std::ifstream is(GetPath());
		is >> data;
		is.close();

		mMaterial.DifuseAlbedo = XMFLOAT4(data["DefuseAlbedo"].get<std::vector<float>>().data());
		mMaterial.FresnelR0 = XMFLOAT3(data["FresnelR0"].get<std::vector<float>>().data());
		mMaterial.Roughness = data["Roughness"];

	}

	void MaterialResource::UploadToGpu(D_GRAPHICS::GraphicsContext& context)
	{
		if (mMaterialConstantsGPU.GetGpuVirtualAddress() == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
		{
			// Initializing Material Constants buffers
			for (size_t i = 0; i < D_RENDERER_FRAME_RESOUCE::gNumFrameResources; i++)
			{
				mMaterialConstantsCPU[i].Create(L"Material Constatns Upload Buffer", sizeof(Material));
			}
			mMaterialConstantsGPU.Create(L"Material Constants GPU Buffer", 1, sizeof(Material));
		}

		// Updating material constnats
		// Mapping upload buffer
		auto& currentMatUploadBuff = mMaterialConstantsCPU[D_RENDERER_DEVICE::GetCurrentResourceIndex()];
		auto matCB = (Material*)currentMatUploadBuff.Map();
		memcpy(matCB, &mMaterial, sizeof(Material));

		currentMatUploadBuff.Unmap();

		// Uploading
		context.TransitionResource(mMaterialConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
		context.GetCommandList()->CopyBufferRegion(mMaterialConstantsGPU.GetResource(), 0, mMaterialConstantsCPU->GetResource(), 0, mMaterialConstantsCPU->GetBufferSize());
		context.TransitionResource(mMaterialConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);
	}

}