#pragma once

#include <ResourceManager/Resource.hpp>
#include <ResourceManager/ResourceRef.hpp>

#include "%%CLASS_NAME%%.generated.hpp"

#ifndef %%NAMESPACE_KEY%%
#define %%NAMESPACE_KEY%% %%NAMESPACE%%
#endif // !%%NAMESPACE_KEY%%

namespace %%NAMESPACE%%
{
	class DClass(Serialize, Resource) %%CLASS_NAME%% : public D_RESOURCE::Resource
	{
		GENERATED_BODY();

		D_CH_RESOURCE_BODY(%%CLASS_NAME%%, %%RESOURCE_NAME%%, %%SUPPORTED_EXT%%);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

	protected:

		virtual bool					WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void					ReadResourceFromFile(D_SERIALIZATION::Json const& j, bool& dirtyDisk) override;
		virtual bool					UploadToGpu() override;
		virtual INLINE void				Unload() override { EvictFromGpu(); }
		virtual INLINE bool				AreDependenciesDirty() const { return false; }


	private:
		%%CLASS_NAME%%(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault = false) :
			D_RESOURCE::Resource(uuid, path, name, id, parent, isDefault) {}
	};
}

%%FILE_GENERATED%%
