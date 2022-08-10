#include "pch.hpp"
#include "MaterialResource.hpp"

#include <Utils/Common.hpp>

#include <nlohmann/json.hpp>

#include <fstream>

namespace Darius::ResourceManager
{
	bool MaterialResource::Save()
	{
		if (mDefault)
			return true;
		if (!SuppoertsExtension(mPath.extension()))
			return false;

		nlohmann::json js = {
			{ "Name", STR_WSTR(mMaterial.Name)},
			{ "DefuseAlbedo", std::vector<float>(4, (float&)mMaterial.DifuseAlbedo)},
			{ "FresnelR0", std::vector<float>(3, (float&)mMaterial.FresnelR0) },
			{ "Roughness", mMaterial.Roughness }
		};

		std::ofstream o(mPath);
		o << std::setw(4) << js << std::endl;

		mDirty = false;
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

		mMaterial.Name = WSTR_STR(data["Name"]);
		mMaterial.DifuseAlbedo = D_MATH::Vector4(data["DefuseAlbedo"].get<std::vector<float>>().data());
		mMaterial.FresnelR0 = D_MATH::Vector3(data["FresnelR0"].get<std::vector<float>>().data());
		mMaterial.Roughness = data["Roughness"];

		return true;
	}

	bool MaterialResource::SuppoertsExtension(std::wstring ext)
	{
		if (ext == L".mat")
			return true;
		return false;
	}

}