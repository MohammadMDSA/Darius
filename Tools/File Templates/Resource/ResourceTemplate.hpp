#pragma once

#include <ResourceManager/ResourceTypes/Resource.hpp>

#ifndef %%NAMESPACE_KEY%%
#define %%NAMESPACE_KEY%% %%NAMESPACE%%
#endif // !%%NAMESPACE_KEY%%

namespace %%NAMESPACE%%
{
	class %%CLASS_NAME%% : public D_RESOURCE::Resource
	{
		D_CH_RESOURCE_BODY(%%CLASS_NAME%%, %%RESOURCE_NAME%%, %%SUPPORTED_EXT%%);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		virtual void					WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void					ReadResourceFromFile(D_SERIALIZATION::Json const& j) override;
		virtual bool					UploadToGpu(void* context) override;
		virtual INLINE void				Unload() override { EvictFromGpu(); }

	protected:
		%%CLASS_NAME%%(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, DResourceId id, bool isDefault = false) :
			Resource(uuid, path, name, id, isDefault) {}

	};
}
