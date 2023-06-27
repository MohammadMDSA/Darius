#include "Graphics/pch.hpp"
#include "FBXPrefabResource.hpp"

#include "Graphics/Geometry/ModelLoader/FbxLoader.hpp"

#include <ResourceManager/ResourceLoader.hpp>
#include <Utils/Common.hpp>

#ifdef _D_EDITOR
#include <Utils/DragDropPayload.hpp>
#include <imgui.h>
#endif

#include "FBXPrefabResource.sgenerated.hpp"

using namespace D_SERIALIZATION;

namespace Darius::Graphics
{

	D_CH_RESOURCE_DEF(FBXPrefabResource);

	FBXPrefabResource::FBXPrefabResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault) :
		Resource(uuid, path, name, id, isDefault),
		mPrefabGameObject(nullptr) { }

	void FBXPrefabResource::WriteResourceToFile(D_SERIALIZATION::Json& j) const
	{
		if (!mPrefabGameObject)
			return;

		j["PrefabUuid"] = D_CORE::ToString(mPrefabGameObject->GetUuid());
	}

	void FBXPrefabResource::ReadResourceFromFile(D_SERIALIZATION::Json const& j)
	{
		D_CORE::Uuid prefabGoUuid;

		if (j.contains("PrefabUuid"))
			prefabGoUuid = D_CORE::FromString(j["PrefabUuid"]);
		else
		{
			prefabGoUuid = D_CORE::GenerateUuid();
			MakeDiskDirty();
		}

		mPrefabGameObject = D_RENDERER_GEOMETRY_LOADER_FBX::LoadScene(GetPath(), prefabGoUuid);
	}

	bool FBXPrefabResource::UploadToGpu()
	{
		return true;
	}

	void FBXPrefabResource::Unload()
	{
		D_WORLD::DeleteGameObject(mPrefabGameObject);
		mPrefabGameObject = nullptr;
		EvictFromGpu();
	}

#ifdef _D_EDITOR
	bool FBXPrefabResource::DrawDetails(float params[])
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
