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
		D_CH_RESOURCE_BODY(%%CLASS_NAME%%, %%RESOURCE_NAME%%, %%SUPPORTED_EXT%%);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

	protected:

		virtual void					WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void					ReadResourceFromFile(D_SERIALIZATION::Json const& j) override;
		virtual bool					UploadToGpu() override;
		virtual INLINE void				Unload() override { EvictFromGpu(); }

	private:
		%%CLASS_NAME%%(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOUCE::DResourceId id, bool isDefault = false) :
			Resource(uuid, path, name, id, isDefault) {}

	public:
		%%CLASS_FOOTER_GENERATED%%
	};
}

%%FILE_GENERATED%%
