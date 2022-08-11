#include "pch.hpp"
#include "MaterialResource.hpp"

#include <Renderer/RenderDeviceManager.hpp>
#include <Utils/Common.hpp>

#include <nlohmann/json.hpp>

#include <fstream>

namespace Darius::ResourceManager
{
	void MaterialResource::UpdateGPU(D_GRAPHICS::GraphicsContext& context)
	{
		if (!mLoaded)
		{
			// Initializing Material Constants buffers
			for (size_t i = 0; i < D_RENDERER_FRAME_RESOUCE::gNumFrameResources; i++)
			{
				mMaterialConstantsCPU[i].Create(L"Material Constatns Upload Buffer", sizeof(Material));
			}
			mMaterialConstantsGPU.Create(L"Material Constants GPU Buffer", 1, sizeof(Material));

			mDirtyGPU = true;
			mLoaded = true;
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

		mDirtyGPU = false;
	}

	bool MaterialResource::Save()
	{
		if (mDefault)
		{
			mDirtyDisk = false;
			return true;
		}
		if (!SuppoertsExtension(mPath.extension()))
			return false;

		nlohmann::json js = {
			{ "DefuseAlbedo", std::vector<float>(4, (float&)mMaterial.DifuseAlbedo)},
			{ "FresnelR0", std::vector<float>(3, (float&)mMaterial.FresnelR0) },
			{ "Roughness", mMaterial.Roughness }
		};

		std::ofstream o(mPath);
		o << std::setw(4) << js << std::endl;

		mDirtyDisk = false;
		return true;
	}

	bool MaterialResource::Load()
	{
		if (mDefault)
			return true;
		if (!SuppoertsExtension(mPath.extension()))
			return false;
		std::ifstream i(mPath);

		auto data = nlohmann::json::parse(i);

		mMaterial.DifuseAlbedo = XMFLOAT4(data["DefuseAlbedo"].get<std::vector<float>>().data());
		mMaterial.FresnelR0 = XMFLOAT3(data["FresnelR0"].get<std::vector<float>>().data());
		mMaterial.Roughness = data["Roughness"];

		mDirtyGPU = true;
		mDirtyDisk = false;
		mLoaded = true;

		return true;
	}

	bool MaterialResource::SuppoertsExtension(std::wstring ext)
	{
		if (ext == L".mat")
			return true;
		return false;
	}

}