#include "pch.hpp"
#include "FBXPrefabResource.hpp"

#include <ResourceManager/ResourceLoader.hpp>
#include <Utils/Common.hpp>

#ifdef _D_EDITOR
#include <Utils/DragDropPayload.hpp>
#include <imgui.h>
#endif

#include "FBXPrefabResource.sgenerated.hpp"

using namespace D_SERIALIZATION;

namespace Darius::Fbx
{

	D_CH_RESOURCE_DEF(FBXPrefabResource);

	FBXPrefabResource::FBXPrefabResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault) :
		D_SCENE::PrefabResource(uuid, path, name, id, parent, isDefault)
		{ }

	void FBXPrefabResource::WriteResourceToFile(D_SERIALIZATION::Json& j) const
	{
		if (!GetPrefabGameObject())
			return;

		D_ASSERT(GetPrefabGameObject()->GetUuid() == mPrefabGoUuid);

		j["PrefabUuid"] = D_CORE::ToString(GetPrefabGameObject()->GetUuid());
	}

	void FBXPrefabResource::ReadResourceFromFile(D_SERIALIZATION::Json const& j, bool& dirtyDisk)
	{
		dirtyDisk = false;
		D_CORE::Uuid prefabUuid;

		if (j.contains("PrefabUuid"))
			mPrefabGoUuid = D_CORE::FromString(j["PrefabUuid"]);
		else
		{
			mPrefabGoUuid = D_CORE::GenerateUuid();
			dirtyDisk = true;
		}

	}

}
