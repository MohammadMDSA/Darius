#pragma once

#include <Scene/Resources/PrefabResource.hpp>

#include "FBXPrefabResource.generated.hpp"

#ifndef D_FBX
#define D_FBX Darius::Fbx
#endif // !D_FBX

namespace Darius::Fbx
{
	class DClass(Serialize, Resource) FBXPrefabResource : public D_SCENE::PrefabResource
	{
		GENERATED_BODY();
		D_CH_RESOURCE_BODY(FBXPrefabResource, "FBX Scene", "");

	public:
		D_CORE::Uuid const&				GetPrefabGameObjectUuid() const { return mPrefabGoUuid; }

	protected:

		virtual void					WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void					ReadResourceFromFile(D_SERIALIZATION::Json const& j, bool& dirtyDisk) override;

	protected:
		FBXPrefabResource(D_CORE::Uuid const& uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault = false);

	private:

		D_CORE::Uuid			mPrefabGoUuid;

	};
}

File_FBXPrefabResource_GENERATED
