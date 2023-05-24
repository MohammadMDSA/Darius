#include "Scene/pch.hpp"
#include "PrefabResource.hpp"

#include "Scene/Scene.hpp"

#include <ResourceManager/ResourceLoader.hpp>

#include <Utils/Common.hpp>

#ifdef _D_EDITOR
#include <Utils/DragDropPayload.hpp>
#include <imgui.h>
#endif

#include <PrefabResource.sgenerated.hpp>

using namespace D_SERIALIZATION;

namespace Darius::Scene
{

	D_CH_RESOURCE_DEF(PrefabResource);

	PrefabResource::PrefabResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault) :
		Resource(uuid, path, name, id, isDefault),
		mPrefabGameObject(nullptr) { }

	void PrefabResource::WriteResourceToFile(D_SERIALIZATION::Json& j) const
	{
		if (!mPrefabGameObject)
			return;

		Json dataJson;

		D_WORLD::DumpGameObject(mPrefabGameObject, dataJson);

		std::ofstream os(GetPath());
		os << dataJson;
		os.close();
	}

	void PrefabResource::ReadResourceFromFile(D_SERIALIZATION::Json const& j)
	{
		Json dataJson;
		std::ifstream is(GetPath());
		is >> dataJson;
		is.close();

		D_WORLD::LoadGameObject(dataJson, &mPrefabGameObject, false);
	}

	bool PrefabResource::UploadToGpu()
	{
		return true;
	}

	void PrefabResource::Unload()
	{
		D_WORLD::DeleteGameObject(mPrefabGameObject);
		mPrefabGameObject = nullptr;
		EvictFromGpu();
	}

	void PrefabResource::CreateFromGameObject(GameObject const* go)
	{
		Json refGoJson;
		D_WORLD::DumpGameObject(go, refGoJson, false);

		D_WORLD::LoadGameObject(refGoJson, &mPrefabGameObject, false);

		D_RESOURCE_LOADER::SaveResource(this);
	}

#ifdef _D_EDITOR
	bool PrefabResource::DrawDetails(float params[])
	{
		if (!mPrefabGameObject)
			return false;

		auto valueChanged = false;

		valueChanged |= mPrefabGameObject->DrawDetails(params);
		
		if (valueChanged)
			MakeDiskDirty();

		return valueChanged;
	}
#endif
}
