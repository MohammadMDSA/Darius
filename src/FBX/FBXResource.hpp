#pragma once

#include "ResourceManager/Resource.hpp"
#include "ResourceManager/ResourceRef.hpp"

#include "FBXResource.generated.hpp"

#ifndef D_FBX
#define D_FBX Darius::Fbx
#endif

namespace Darius::Fbx
{
	class DClass(Serialize, Resource) FBXResource : public D_RESOURCE::Resource
	{
		GENERATED_BODY();
		D_CH_RESOURCE_BODY(FBXResource, "FBX Resource", ".fbx");

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		virtual void					ReadResourceFromFile(D_SERIALIZATION::Json const&, bool& dirtyDisk) override;
		virtual bool					WriteResourceToFile(D_SERIALIZATION::Json & j) const override { return true; }

		INLINE virtual bool				AreDependenciesDirty() const override { return false; }

		static D_RESOURCE::SubResourceConstructionData CanConstructFrom(D_RESOURCE::ResourceType type, D_FILE::Path const& path);

	protected:
		INLINE virtual bool				UploadToGpu() override { return true; }
		virtual void					Unload() override;


	private:
		FBXResource(D_CORE::Uuid const& uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault = false) :
			Resource(uuid, path, name, id, parent, isDefault)
		{}


#if _D_EDITOR
		D_CONTAINERS::DVector<D_RESOURCE::ResourceDataInFile> mResourceDataInFile;
#endif // _D_EDITOR

	};
}
