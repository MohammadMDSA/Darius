#pragma once

#include <ResourceManager/ResourceTypes/Resource.hpp>

#ifndef %%NAMESPACE_KEY%%
#define %%NAMESPACE_KEY%% %%NAMESPACE%%
#endif // !%%NAMESPACE_KEY%%

using namespace D_RESOURCE;

namespace %%NAMESPACE%%
{
	class %%CLASS_NAME%% : public D_RESOURCE::Resource
	{
		D_CH_RESOURCE_BODY(%%CLASS_NAME%%, %%RESOURCE_NAME%%, %%SUPPORTED_EXT%%);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		virtual void					WriteResourceToFile() const override;
		virtual void					ReadResourceFromFile() override;
		virtual bool					UploadToGpu(D_GRAPHICS::GraphicsContext& context);
		virtual INLINE void				Unload() override { EvictFromGpu(); }

	protected:
		%%CLASS_NAME%%(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, DResourceId id, bool isDefault = false) :
			Resource(uuid, path, name, id, isDefault) {}

	};
}
